#include <cstdio>
#include <vector>
#include <iostream>
#include <cmath>
#include <fstream>
#include <numeric>
#include <execution>
#include <algorithm>

#include "stl_io.hh"
#include "vec3.hh"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> TreeK;

typedef TreeK::Point_3 CGALPoint3;
typedef TreeK::Triangle_3 CGALTriangle3;
typedef TreeK::Segment_3 CGALSegment3;
typedef TreeK::Vector_3 CGALVector3;
typedef TreeK::Ray_3 CGALRay3;

typedef std::vector<CGALTriangle3>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<TreeK, Iterator> Primitive;
typedef CGAL::AABB_traits<TreeK, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;

using namespace mp::io::stl;

const CGALVector3 SPHERE_SAMPLES[] =
    {
        {0.7119194972109728, -0.696039979418038, -0.09326830406205966},
        {0.22064732904544748, 0.9504192097730052, -0.2191302851720096},
        {-0.5356309362690355, 0.7719351258159894, -0.34236772868220067},
        {0.8755608617563906, 0.06539669228746889, -0.47866110140501794},
        {-0.8949236186386672, -0.185304025353087, -0.4059238044148381},
        {-0.8649100833758778, -0.06332101677235083, 0.49791665618820957},
        {-0.4649455244931791, -0.49265853053684555, -0.7356039909781884},
        {-0.2283466732532642, -0.9312775816598549, 0.2838659238301411},
        {-0.9146482241310125, 0.33271779735507045, 0.22960290376468562},
        {0.8089945757237522, -0.11066089233984837, -0.5773057624483708},
        {0.6682918549871739, 0.7422493482365842, 0.04951668001964782},
        {-0.7310673858339648, -0.00127077089847803, -0.6823040836102452},
        {0.3621005525029778, 0.2035172544824478, 0.9096504367090497},
        {0.3975543416344575, 0.9135800358327174, -0.08556905734834208},
        {0.9848243629681159, -0.13494468190569925, -0.10913710152745715},
        {0.5149819987650766, 0.7559866766379911, 0.4040763364684601},
        {-0.19839565698572814, 0.14313186521205018, -0.9696145793304269},
        {-0.32955758208532426, 0.8260710425106442, -0.45716346399887664},
        {0.3845794336953236, -0.7214139385637832, 0.5758998076267035},
        {-0.45771111504262496, -0.6469300048634257, -0.6099033562572393},
        {-0.5405149810739388, -0.02348065942337531, 0.8410066669579279},
        {0.39376816545300763, 0.8680307249955945, 0.3024389067884541},
        {0.4109016715024701, -0.10067897162602654, 0.9061035045891843},
        {-0.8659087371223363, 0.29853675589874334, 0.40134506892779287},
        {0.9472351739666208, -0.19946929717971534, -0.2509133808370052},
        {-0.5237495909737049, 0.6222259100109125, 0.5818258183219172},
        {0.4659012086517155, 0.7700429797363834, 0.4358553351005152},
        {-0.8963045960149939, 0.3606494790872522, 0.2580116749228458},
        {0.7987007811235346, 0.06506421553953189, 0.5982003929193636},
        {-0.2846800038648522, -0.9586211629258707, -0.001661743108563929},
        {0.6792296240630873, -0.3006616166558325, 0.6695145331245755},
        {0.6617191697278286, -0.08671083592587477, 0.7447207337974076},
        {0.17538653718883934, 0.4026760023890149, -0.8983827690204826},
        {0.19816440534834942, 0.8464314747620121, -0.49425158268349567},
        {0.07136153823859155, 0.9535262136551049, 0.2927375799804004},
        {0.9064350487979264, -0.41978189190241216, 0.04646144144914488},
        {-0.3413351941340609, 0.8662954903317545, 0.3647223720534949},
        {0.2713432251407809, 0.29875356382945467, -0.9149421633466235},
        {-0.388705123887701, -0.6926968011193344, 0.6075191094792725},
        {0.7995702944367864, 0.6005099051183451, -0.008671684324687012},
        {-0.909180463409936, 0.406054289063773, -0.09225399333691775},
        {-0.8261322924765581, -0.5369882612232836, -0.17073090709013838},
        {0.5218436590381971, 0.30578879165329936, -0.7963494273375482},
        {0.9399573085556245, 0.17073473524388266, 0.29551634180543696},
        {-0.006734372319091334, 0.7505572707967827, 0.6607710885651346},
        {-0.5974001776609363, 0.35555291713480786, 0.7188151019891069},
        {0.8541466993276515, 0.3391634680177603, -0.3942100429845037},
        {0.9226605138101509, 0.3364507034392194, 0.18841045727571126},
        {-0.9913211214976476, -0.13071645742746724, 0.013987202374289964},
        {0.049048781034101194, -0.0564908377202492, -0.9971975743716655},
        {-0.5872206250143815, 0.5639214070089813, 0.5806587502782765},
        {-0.5862487186773326, 0.6745160947981155, -0.44870979230175645},
        {0.005939432451019611, 0.7432400910139246, -0.6689984232057447},
        {0.9780029682957686, 0.18494032404517774, -0.09647419627408427},
        {-0.18757945797641637, 0.8344397802302147, -0.5181932073219648},
        {0.9270663958362754, -0.36707783142619727, -0.07616930737888317},
        {0.3269709524229819, -0.07235255937756033, 0.9422606345502951},
        {-0.9877803085354633, 0.0713806958094671, 0.13854550997900072},
        {-0.559474194178625, 0.16506303094730973, 0.8122455428395194},
        {-0.9813216090580165, 0.18795395116729863, -0.04100258328918449},
        {-0.7613256126229363, -0.4116759677229068, 0.5009053894336923},
        {-0.02956201986803973, -0.2460321967207496, 0.9688107375324058},
        {-0.5901272940081926, -0.7991250977640666, -0.11466845682287374},
        {0.986061002232592, 0.11435618320165206, -0.12085678814036993},
        {0.16639752209393674, -0.8809104291821975, -0.4430673542465462},
        {-0.9839533902957053, 0.1759353016855809, -0.029706823229539436},
        {-0.513649926652476, 0.7366415146750338, 0.43991252733603536},
        {-0.38688589913297455, -0.9117992076084812, -0.13762814412980906},
        {0.1761626316755963, 0.11113820181628334, 0.9780669850772856},
        {-0.8498574014432385, 0.2075334200445036, 0.48442984711594494},
        {-0.8465921083791693, -0.5018228671046348, -0.17735730061375277},
        {-0.6636083631606712, -0.4920067936633031, 0.5635186379636175},
        {-0.42388966869553873, 0.31875423689704785, 0.8477695944260841},
        {0.4784628385580975, 0.0790319869339507, -0.8745440281427801},
        {0.2012151457646944, -0.7604120094060234, -0.6174836362738583},
        {-0.33637631542925905, 0.4563601508633176, -0.8237635504938572},
        {-0.7245923883643549, 0.010938523987038432, -0.6890908644129776},
        {0.0936642904364271, -0.9926797380492642, -0.07624918598571728},
        {0.2599166670958361, -0.9437533103402327, -0.20438447932180326},
        {-0.0219723263722354, 0.2512561325317029, -0.9676712110727503},
        {0.8521506099759558, -0.364694485494494, 0.37528292016492415},
        {-0.08521445421474597, 0.1641034226919317, 0.9827555970095903},
        {0.14949193549147718, -0.535293436721412, -0.8313321224552745},
        {-0.23044509487912984, 0.405170194371211, 0.8847215221974336},
        {0.9016409315877965, -0.291451803054214, 0.3195300877569094},
        {0.7942160709615016, -0.5162569017474413, 0.3204678517801869},
        {0.5145415369473799, -0.6740715368555226, -0.529976008850459},
        {0.17135481612739378, -0.10259083071693398, 0.979853381094721},
        {0.5417271296512001, 0.26324364159144137, 0.7982696926237076},
        {-0.4785822866266422, -0.8091817894148106, -0.3408574872386696},
        {0.12696481941328527, 0.6689996649507624, 0.7323382981430915},
        {0.46112853580780466, -0.5783039028491751, 0.6729970797954195},
        {0.5210058992204449, 0.30577216629098103, 0.7969041569092334},
        {-0.37005132775577865, 0.8021232291511622, 0.46867935743138656},
        {-0.3999252778787448, -0.8758713229889757, 0.27001703220195217},
        {0.8156699228976728, -0.07501910771353533, -0.5736329055746101},
        {-0.010263865153361465, -0.8740152656928246, -0.4857900456040807},
        {0.6705967073187743, 0.5144198638460197, 0.5344831707489588},
        {-0.3413363259772129, -0.6017608162744591, -0.7220619312531817},
        {-0.34063976582192357, -0.7657244733148851, -0.5455552959209788},
};

static std::vector<CGALTriangle3> load_stl(const char *filepath)
{
    std::vector<Triangle> tris;
    read_stl(filepath, tris);

    std::vector<CGALTriangle3> out;
    for (auto const &t : tris)
    {
        CGALPoint3 p1(t.v1[0], t.v1[1], t.v1[2]);
        CGALPoint3 p2(t.v2[0], t.v2[1], t.v2[2]);
        CGALPoint3 p3(t.v3[0], t.v3[1], t.v3[2]);

        out.push_back({p1, p2, p3});
    }

    return out;
}

static double tet_solid_angle(const CGALPoint3 &origin, const CGALPoint3 &a, const CGALPoint3 &b, const CGALPoint3 &c)
{
    auto av = a - origin;
    auto bv = b - origin;
    auto cv = c - origin;

    auto al = sqrt(av.squared_length());
    auto bl = sqrt(bv.squared_length());
    auto cl = sqrt(cv.squared_length());

    auto numerator = CGAL::determinant(av, bv, cv);
    auto av_dot_bv = CGAL::scalar_product(av, bv);
    auto av_dot_cv = CGAL::scalar_product(av, cv);
    auto bv_dot_cv = CGAL::scalar_product(bv, cv);

    auto denominator = al * bl * cl + av_dot_bv * cl + av_dot_cv * bl + bv_dot_cv * al;

    return atan2(numerator, denominator);
}

static bool is_inside(const Tree &tree, const CGALPoint3 &query_point, float hole_tolerance)
{
    double w = 0.0;
    int n = 0;
    for (int i = 0; i < sizeof(SPHERE_SAMPLES) / sizeof(SPHERE_SAMPLES[0]); i++)
    {
        CGALRay3 ray(query_point, SPHERE_SAMPLES[i]);
        auto intersected_triangle = tree.first_intersected_primitive(ray);
        if (intersected_triangle)
        {
            auto it = boost::get<Iterator>(intersected_triangle);
            w += tet_solid_angle(query_point, it->vertex(0), it->vertex(1), it->vertex(2));
            n += 1;
        }
    }

    hole_tolerance = std::clamp(hole_tolerance, 0.0f, 1.0f);
    return (w >= ((1.0f - hole_tolerance) * 2.0 * M_PI)) || (n == 100);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        puts("Monte Carlo Winding Numbers\n"
             "Usage: mcwn grid_step hole_tolerance input_filepath.stl output_filepath.pts\n"
             "Example: mcwn bunny.stl 5.0 1.0 bunny_points.pts\n"
             "Generates points inside the volume of an oriented triangle soup by filtering bounding box grid points.\n"
             "Outputs a binary file containing N * 3 doubles.");
        return 1;
    }

    float grid_step = atof(argv[1]);
    if (grid_step <= 0.0f)
    {
        puts("ERROR: Grid step must be a positive number.");
        return 1;
    }
    float hole_tolerance = atof(argv[2]);
    if (hole_tolerance > 1.0f || hole_tolerance < 0.0f)
    {
        puts("ERROR: Hole tolerance must be between 0.0 and 1.0 inclusive.");
        return 1;
    }
    char *input_filepath = argv[3];
    char *output_filepath = argv[4];

    std::cout << "CGAL Version: " << CGAL_VERSION_STR << std::endl;

    auto tri_soup = load_stl(input_filepath);
    Tree tree(tri_soup.begin(), tri_soup.end());

    // Calculate bounding box
    Vec3 bb_min(INFINITY, INFINITY, INFINITY);
    Vec3 bb_max(-INFINITY, -INFINITY, -INFINITY);
    for (auto const &tri : tri_soup)
    {
        for (int i = 0; i < 3; i++)
        {
            auto v = tri.vertex(i);
            auto vec = Vec3(v[0], v[1], v[2]);
            bb_min.min(vec);
            bb_max.max(vec);
        }
    }

    // Generate and filter grid points and write them to a file
    std::ofstream file(output_filepath, std::ios::binary);

    Vec3 bb_dims = bb_max - bb_min;
    int num_x = static_cast<int>(bb_dims.x / grid_step);
    int num_y = static_cast<int>(bb_dims.y / grid_step);
    int num_z = static_cast<int>(bb_dims.z / grid_step);

    int num_points = num_x * num_y * num_z;
    printf("Number of grid points before filtering = %d\n", num_points);

#pragma omp parallel for collapse(3)
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            for (int k = 0; k < num_z; k++)
            {
                CGALPoint3 query_point(i * grid_step + bb_min.x, j * grid_step + bb_min.y, k * grid_step + bb_min.z);
                if (is_inside(tree, query_point, hole_tolerance))
                {
#pragma omp critical
                    {
                        file.write(reinterpret_cast<const char *>(&query_point.x()), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&query_point.y()), sizeof(double));
                        file.write(reinterpret_cast<const char *>(&query_point.z()), sizeof(double));
                    }
                }
            }
        }
    }

    return 0;
}