/* This program takes as input a 3D shape, and produces two outputs:
 *
 *   i)  a dense tetrahedralization of such shape, using the
 *       Tetgen library (http://wias-berlin.de/software/tetgen/)
 *
 *   ii) a dense polyhedralization of the same shape, defined
 *       as the dual of the previously generated tetrahedral mesh
*/

#include <QApplication>
#include <QBoxLayout>
#include <cinolib/meshes/meshes.h>
#include <cinolib/dual_mesh.h>
#include <cinolib/tetgen_wrap.h>
#include <cinolib/gui/qt/qt_gui_tools.h>
#include <cinolib/vector_serialization.h>
#include <cinolib/profiler.h>

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

int main(int argc, char **argv)
{
    using namespace cinolib;

    QApplication app(argc, argv);

    std::string s = (argc==2) ? std::string(argv[1]) : std::string(DATA_PATH) + "/3holes.obj";
    DrawableTrimesh<> m_in(s.c_str());

    // make tetmesh
    std::vector<uint>   edges, tets;
    std::vector<double> verts;
    double vol_thresh = 0.01 * m_in.bbox().diag(); // force tets to be smaller than 5% of bbox diag
    char opt[100];
    sprintf(opt, "YQqa%f", vol_thresh);
    tetgen_wrap(serialized_xyz_from_vec3d(m_in.vector_verts()), serialized_vids_from_polys(m_in.vector_polys()), edges, opt, verts, tets);
    DrawableTetmesh<> m_tet(verts, tets);

    Profiler profiler;

    // make polygon mesh
    DrawablePolyhedralmesh<> m_poly;
    profiler.push("Dualize mesh");
    dual_mesh(m_tet, m_poly, true);
    profiler.pop();
    m_poly.updateGL();

    QWidget  gui;
    QHBoxLayout layout;
    gui.setLayout(&layout);
    GLcanvas gui_in(&gui);
    GLcanvas gui_tet(&gui);
    GLcanvas gui_poly(&gui);
    layout.addWidget(&gui_in);
    layout.addWidget(&gui_tet);
    layout.addWidget(&gui_poly);
    gui_in.push_obj(&m_in);
    gui_tet.push_obj(&m_tet);
    gui_poly.push_obj(&m_poly);
    gui.show();
    gui.resize(800,600);
    m_in.show_wireframe(true);

    std::string pov("0.217281 -0.976103 0.00348926 0 0.94495 0.209448 -0.2514 0 0.244662 0.0579212 0.967876 0 -0.747865 0.366815 -2.40522 1 1.32704 2.47434");
    gui_in.deserialize_POV(pov);
    gui_tet.deserialize_POV(pov);
    gui_poly.deserialize_POV(pov);

    // show interior
    SlicerState slice_params;
    slice_params.Z_thresh = 0.6;
    m_in.slice(slice_params);
    m_tet.slice(slice_params);
    m_poly.slice(slice_params);

    // CMD+1 to show in   mesh controls.
    // CMD+2 to show tet  mesh controls.
    // CMD+3 to show poly mesh controls.
    SurfaceMeshControlPanel<DrawableTrimesh<>>        panel_in  (&m_in,   &gui_in);
    VolumeMeshControlPanel<DrawableTetmesh<>>         panel_tet (&m_tet,  &gui_tet);
    VolumeMeshControlPanel<DrawablePolyhedralmesh<>>  panel_poly(&m_poly, &gui_poly);
    QApplication::connect(new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_1), &gui_in),   &QShortcut::activated, [&](){panel_in.show();});
    QApplication::connect(new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_2), &gui_tet),  &QShortcut::activated, [&](){panel_tet.show();});
    QApplication::connect(new QShortcut(QKeySequence(Qt::CTRL+Qt::Key_3), &gui_poly), &QShortcut::activated, [&](){panel_poly.show();});


    return app.exec();
}
