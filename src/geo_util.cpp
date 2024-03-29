//
// Created by Charles Du on 11/9/22.
//

#include "geo_util.h"
#include <Eigen/Geometry>
#include <set>
#include <limits>

Eigen::Vector2d rotate_90deg(const Eigen::Vector2d &vec) {
    return {-(vec.y()), vec.x()};
}


double angle_mod_2PI(double a) {
    if (a >= 0) {
        return fmod(a, 2*M_PI);
    } else {
        return 2*M_PI + fmod(a, 2*M_PI);
    }
}

double compute_vector_angle(const Eigen::Vector2d &p) {
    double x = p.x(), y = p.y();
    if (x == 0) {
        if (y >=0) {
            return M_PI/2;
        } else {
            return 3*M_PI/2;
        }
    } else {
        if (x > 0) {
            if (y >= 0) {
                return asin(y/sqrt(x*x+y*y));
            } else {
                return 2*M_PI + asin(y/sqrt(x*x+y*y));
            }
        } else { // x < 0
            return M_PI - asin(y/sqrt(x*x+y*y));
        }
    }
}

double compute_rotation_angle(double a1, double a2) {
    double angle1 = angle_mod_2PI(a1);
    double angle2 = angle_mod_2PI(a2);
    if (angle1 <= angle2) {
        return angle2 - angle1;
    } else {
        return angle2 + 2*M_PI - angle1;
    }
}

bool is_angle_between_ccw(double a, double a1, double a2) {
    double angle = angle_mod_2PI(a);
    double angle1 = angle_mod_2PI(a1);
    double angle2 = angle_mod_2PI(a2);

    if (angle1 <= angle2) {
        return (angle >= angle1) && (angle < angle2);
    } else {
        return (angle < angle2) || (angle >= angle1);
    }
}

bool is_angle_between_cw(double a, double a1, double a2) {
    double angle = angle_mod_2PI(a);
    double angle1 = angle_mod_2PI(a1);
    double angle2 = angle_mod_2PI(a2);

    if (angle2 <= angle1) {
        return (angle2 < angle) && (angle <= angle1);
    } else {
        return (angle <= angle1) || (angle > angle2);
    }
}

bool is_angle_between(double a, double a1, double a2) {
    if (a1 < a2) {
        return is_angle_between_ccw(a,a1,a2);
    } else {
        return is_angle_between_cw(a,a1,a2);
    }
}

double compute_total_signed_area(const std::vector<Point> &vertices,
                                 const std::vector<std::pair<size_t, size_t>> &edges) {
    double area = 0;
    for (const auto & e : edges) {
        auto i = e.first;
        auto j = e.second;
        area += vertices[i].x() * vertices[j].y() - vertices[i].y() * vertices[j].x();
    }
    area /= 2;

    return area;
}

double compute_total_signed_volume(const std::vector<Point3D> &vertices,
                                   const std::vector<std::array<int,3>> &triangles)
{
    Point3D origin(0,0,0);
    double volume = 0;
    for (const auto & f : triangles) {
        volume += compute_tet_signed_volume(vertices[f[0]], vertices[f[1]], vertices[f[2]], origin);
    }
    return volume;
}

double compute_total_signed_area_with_gradient(const std::vector<Point> &vertices,
                                               const std::vector<std::pair<size_t, size_t>> &edges,
                                               Eigen::Matrix2Xd &dArea_dv) {
    // note: we don't reset dArea_dv at the beginning
    double area = 0;
    for (const auto & e : edges) {
        auto i = e.first;
        auto j = e.second;
        area += vertices[i].x() * vertices[j].y() - vertices[i].y() * vertices[j].x();
        // update derivatives
        dArea_dv.col(i) += 0.5 * Eigen::Vector2d(vertices[j].y(), -(vertices[j].x()));
        dArea_dv.col(j) += 0.5 * Eigen::Vector2d(-(vertices[i].y()), vertices[i].x());
    }
    area /= 2;

    return area;
}


double compute_total_signed_volume_with_gradient(const std::vector<Point3D> &vertices,
                                                 const std::vector<std::array<int,3>> &triangles,
                                                 Eigen::Matrix3Xd &dVolume_dv)
{
    // note: we don't reset dVolume_dv at the beginning
    double volume = 0;
    Point3D origin(0,0,0);
    Eigen::Matrix3Xd grad;
    for (const auto& f: triangles) {
        auto i = f[0];
        auto j = f[1];
        auto k = f[2];
        volume += compute_tet_signed_volume_with_gradient(vertices[i], vertices[j], vertices[k], origin, grad);
        // update derivatives
        dVolume_dv.col(i) += grad.col(0);
        dVolume_dv.col(j) += grad.col(1);
        dVolume_dv.col(k) += grad.col(2);
    }
    return volume;
}

double compute_Heron_tri_area(double d1, double d2, double d3) {
    // sort d1,d2,d3 as a >= b >= c
    double a, b, c;
    if (d1 > d2) {
        a = d1;
        b = d2;
    }
    else {
        a = d2;
        b = d1;
    }
    c = d3;
    if (d3 > b) {
        c = b;
        b = d3;
        if (d3 > a) {
            b = a;
            a = d3;
        }
    }

    a = sqrt(a);
    b = sqrt(b);
    c = sqrt(c);

    return 0.25 * sqrt(abs((a + (b + c)) * (c - (a - b)) * (c + (a - b)) * (a + (b - c))));
}

double compute_tri_aspect_ratio(double d1, double d2, double d3) {
    double a = sqrt(d1);
    double b = sqrt(d2);
    double c = sqrt(d3);
    double s = (a+b+c)/2;
    if (s-a == 0 || s-b == 0 || s-c == 0) {
        return std::numeric_limits<double>::infinity();
    } else {
//        return a * b * c / (8 * (s - a) * (s - b) * (s - c));
        return (a/(s-a)) * (b/(s-b)) * (c/(s-c)) / 8;
    }
}

double compute_tri_signed_area(const Point &p1, const Point &p2, const Point &p3) {
    return 0.5 * (p3(0) * (p1(1) - p2(1)) + p1(0) * (p2(1) - p3(1)) + p2(0) * (p3(1) - p1(1)));
}

void compute_signed_tri_areas(const Eigen::Matrix2Xd &V, const Eigen::Matrix3Xi &F, Eigen::VectorXd &areaList) {
    int nf = F.cols();
    areaList.resize(nf);
    for (int i = 0; i < nf; ++i) {
        const Point &p1 = V.col(F(0, i));
        const Point &p2 = V.col(F(1, i));
        const Point &p3 = V.col(F(2, i));
        areaList(i) = compute_tri_signed_area(p1, p2, p3);
    }
}

void compute_signed_tet_volumes(const Eigen::Matrix3Xd &V, const Eigen::Matrix4Xi &Tets,
                                Eigen::VectorXd &volumeList) {
    int nT = Tets.cols();
    volumeList.resize(nT);
    for (int i = 0; i < nT; ++i) {
        const Point3D &p1 = V.col(Tets(0, i));
        const Point3D &p2 = V.col(Tets(1, i));
        const Point3D &p3 = V.col(Tets(2, i));
        const Point3D &p4 = V.col(Tets(3, i));
        volumeList(i) = compute_tet_signed_volume(p1, p2, p3, p4);
    }
}

double compute_min_signed_mesh_area(const Eigen::Matrix2Xd &V, const Eigen::Matrix3Xi &F) {
    Eigen::VectorXd areaList;
    compute_signed_tri_areas(V, F, areaList);
    return areaList.minCoeff();
}

double compute_min_signed_mesh_volume(const Eigen::Matrix3Xd &V, const Eigen::Matrix4Xi &Tets)
{
    Eigen::VectorXd volumeList;
    compute_signed_tet_volumes(V, Tets, volumeList);
    return volumeList.minCoeff();
}


double compute_total_signed_mesh_area(const Eigen::Matrix2Xd &V, const Eigen::Matrix3Xi &F) {
    Eigen::VectorXd areaList;
    compute_signed_tri_areas(V, F, areaList);
    return areaList.sum();
}

double compute_total_signed_mesh_volume(const Eigen::Matrix3Xd &V,const Eigen::Matrix4Xi &Tets) {
    Eigen::VectorXd volumeList;
    compute_signed_tet_volumes(V, Tets, volumeList);
    return volumeList.sum();
}

double compute_total_unsigned_area(const Eigen::MatrixXd &V, const Eigen::Matrix3Xi &F) {
    int nF = F.cols();
    Eigen::Matrix3Xd D(3, nF);
    compute_squared_edge_Length(V, F, D);

    // compute triangle areas
    double area = 0;
    for (int i = 0; i < nF; ++i) {
        area += compute_Heron_tri_area(D(0, i), D(1, i), D(2, i));
    }
    return area;
}

double compute_total_unsigned_volume(const Eigen::Matrix3Xd &V, const Eigen::Matrix4Xi &Tets) {
    Eigen::VectorXd volumeList;
    compute_signed_tet_volumes(V, Tets, volumeList);
    return volumeList.cwiseAbs().sum();
}


void compute_squared_edge_Length(const Eigen::MatrixXd &V, const Eigen::Matrix3Xi &F,
                                 Eigen::Matrix3Xd &D) {
    auto nf = F.cols();
    // int simplexSize = F.rows();
    // int n_edge = simplexSize * (simplexSize-1) / 2;
    int n_edge = 3;

    D.resize(n_edge, nf);
    for (int i = 0; i < nf; ++i) {
        auto v1 = V.col(F(0, i));
        auto v2 = V.col(F(1, i));
        auto v3 = V.col(F(2, i));
        auto e1 = v2 - v3;
        auto e2 = v3 - v1;
        auto e3 = v1 - v2;
        D(0, i) = e1.squaredNorm();
        D(1, i) = e2.squaredNorm();
        D(2, i) = e3.squaredNorm();
    }
}

void
extract_mesh_boundary_edges(const Eigen::Matrix3Xi &faces, std::vector<std::pair<size_t, size_t>> &boundary_edges)
{
    typedef std::pair<size_t,size_t> Edge;

    // collect all half edges
    std::set<Edge> half_edges;
    for (int i = 0; i < faces.cols(); ++i) {
        auto v1 = faces(0,i);
        auto v2 = faces(1,i);
        auto v3 = faces(2,i);

        half_edges.emplace(v1,v2);
        half_edges.emplace(v2,v3);
        half_edges.emplace(v3,v1);
    }

    // boundary edges are those without an opposite half edge
    boundary_edges.clear();
    for (const auto & he : half_edges) {
        Edge reverse_he(he.second, he.first);
        if (half_edges.find(reverse_he) == half_edges.end()) {
            boundary_edges.emplace_back(he);
        }
    }
}

void extract_mesh_boundary_vertices(const Eigen::Matrix3Xi &faces, std::vector<size_t> &boundary_vertices) {
    std::vector<std::pair<size_t, size_t>> boundary_edges;
    extract_mesh_boundary_edges(faces, boundary_edges);
    // create a set of size_t to collect indices of boundary vertices, then convert the set to a vector
    std::set<size_t> boundary_vertex_set;
    for (const auto & edge : boundary_edges) {
        boundary_vertex_set.insert(edge.first);
        boundary_vertex_set.insert(edge.second);
    }
    boundary_vertices.assign(boundary_vertex_set.begin(), boundary_vertex_set.end());
}

void extract_mesh_boundary_triangles(const Eigen::Matrix4Xi &tets,
                                     std::vector<std::array<int,3>>& boundary_triangles)
{
    typedef std::array<int ,3> Triangle;

    // collect all half faces
    std::set<Triangle> half_faces;

    for (int i = 0; i < tets.cols(); ++i) {
        auto v1 = tets(0,i);
        auto v2 = tets(1,i);
        auto v3 = tets(2,i);
        auto v4 = tets(3,i);

        half_faces.emplace(Triangle {v4, v3, v2});
        half_faces.emplace(Triangle {v1, v3, v4});
        half_faces.emplace(Triangle {v1, v4, v2});
        half_faces.emplace(Triangle {v1, v2, v3});
    }

    // boundary faces are those without an opposite half face
    boundary_triangles.clear();
    for (const auto & hf : half_faces) {
        Triangle opposite_hf1 {hf[0], hf[2], hf[1]};
        Triangle opposite_hf2 {hf[2], hf[1], hf[0]};
        Triangle opposite_hf3 {hf[1], hf[0], hf[2]};
        if (half_faces.find(opposite_hf1) == half_faces.end() &&
            half_faces.find(opposite_hf2) == half_faces.end() &&
            half_faces.find(opposite_hf3) == half_faces.end()
                ) {
            boundary_triangles.emplace_back(hf);
        }
    }
}

void extract_mesh_boundary_vertices(const Eigen::Matrix4Xi &tets, std::vector<size_t> &boundary_vertices) {
    std::vector<std::array<int,3>> boundary_triangles;
    extract_mesh_boundary_triangles(tets, boundary_triangles);
    // create a set of size_t to collect indices of boundary vertices, then convert the set to a vector
    std::set<size_t> boundary_vertex_set;
    for (const auto & tri : boundary_triangles) {
        boundary_vertex_set.insert(tri[0]);
        boundary_vertex_set.insert(tri[1]);
        boundary_vertex_set.insert(tri[2]);
    }
    boundary_vertices.assign(boundary_vertex_set.begin(), boundary_vertex_set.end());
}


void compute_winded_interior_vertices(const Eigen::Matrix2Xd &V, const Eigen::Matrix3Xi &F,
                                      const std::vector<bool> &is_boundary_vertex,
                                      std::vector<size_t> &winded_vertices) {
    // compute total signed angle at each vertex
    Eigen::VectorXd vert_angles = Eigen::VectorXd::Zero(V.cols());
    for (int i = 0; i < F.cols(); ++i) {
        auto i1 = F(0,i);
        auto i2 = F(1,i);
        auto i3 = F(2,i);
        const auto &p1 = V.col(i1);
        const auto &p2 = V.col(i2);
        const auto &p3 = V.col(i3);
        vert_angles(i1) += compute_vec_vec_angle(p2-p1,p3-p1);
        vert_angles(i2) += compute_vec_vec_angle(p3-p2,p1-p2);
        vert_angles(i3) += compute_vec_vec_angle(p1-p3,p2-p3);
    }

    // find interior vertices with total angle != 2 pi
    winded_vertices.clear();
    for (size_t i = 0; i < V.cols(); ++i) {
        if ((!is_boundary_vertex[i]) && (round(vert_angles(i)/(2*M_PI)) != 1)) {
            winded_vertices.push_back(i);
        }
    }
}

double compute_vec_vec_angle(const Eigen::Vector2d &vec1, const Eigen::Vector2d &vec2) {
    double cos = vec1.dot(vec2);
    double sin = vec1.x() * vec2.y() - vec1.y() * vec2.x();
    double angle = compute_vector_angle(Point(cos,sin));
    if (angle > M_PI) {
        angle = angle - 2*M_PI;
    }
    return angle;
}

double compute_tet_signed_volume(const Point3D &p1, const Point3D &p2, const Point3D &p3, const Point3D &p4) {
    return (p4(0)*(p3(1)*(p1(2) - p2(2)) + p2(1)*(p3(2) - p1(2)) + p1(1)*(p2(2) - p3(2))) +
            p3(0)*(p4(1)*(p2(2) - p1(2)) + p1(1)*(p4(2) - p2(2)) + p2(1)*(p1(2) - p4(2))) +
            p1(0)*(p4(1)*(p3(2) - p2(2)) + p2(1)*(p4(2) - p3(2)) + p3(1)*(p2(2) - p4(2))) +
            p2(0)*(p4(1)*(p1(2) - p3(2)) + p3(1)*(p4(2) - p1(2)) + p1(1)*(p3(2) - p4(2))))/6;
}

double
compute_tet_signed_volume_with_gradient(const Point3D &p1, const Point3D &p2, const Point3D &p3, const Point3D &p4,
                                        Eigen::Matrix3Xd &grad) {
    Eigen::Vector3d e12 = p2 - p1;
    Eigen::Vector3d e13 = p3 - p1;
    Eigen::Vector3d e14 = p4 - p1;
    Eigen::Vector3d e23 = p3 - p2;
    Eigen::Vector3d e24 = p4 - p2;
    // gradient
    grad.resize(3,4);
    grad.col(0) = e24.cross(e23);
    grad.col(1) = e13.cross(e14);
    grad.col(2) = e14.cross(e12);
    grad.col(3) = e12.cross(e13);
    grad /= 6;
    // signed volume
    return (e12.cross(e13)).dot(e14)/6;
}


