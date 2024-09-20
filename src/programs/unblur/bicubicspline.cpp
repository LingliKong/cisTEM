#include <dlib/dlib/queue.h>
#include "../../core/core_headers.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <dlib/dlib/matrix.h>
#include "bicubicspline.h"

using namespace std;
using namespace dlib;
using namespace cistem;

void bicubicspline::InitializeSpline(int row_no, int column_no, float spline_patch_dimy, float spline_patch_dimx) {
    this->m                  = row_no;
    this->n                  = column_no;
    this->spline_patch_dim_x = spline_patch_dimx;
    this->spline_patch_dim_y = spline_patch_dimy;
    this->total              = (this->m + 2) * (this->n + 2);
    this->totaldim           = (this->m + 2) * (this->n + 2);
    this->Qz2d.set_size(this->m + 2, this->n + 2);
    // CalcPhi( );
}

void bicubicspline::InitializeSplineForwardModel(int row_no, int column_no, float spline_patch_dimy, float spline_patch_dimx) {
    this->m                  = row_no;
    this->n                  = column_no;
    this->spline_patch_dim_x = spline_patch_dimx;
    this->spline_patch_dim_y = spline_patch_dimy;
    this->total              = (this->m + 2) * (this->n + 2);
    this->totaldim           = (this->m + 2) * (this->n + 2);
    // wxPrintf("phi1?\n");
    CalcPhi( );
    // wxPrintf("phi?\n");
}

void bicubicspline::UpdateSpline(matrix<double> z_on_knot) {
    this->z_on_knot = z_on_knot;
    this->Qz2d      = CalcQz(z_on_knot);
}

void bicubicspline::UpdateSpline1dControlPoints(matrix<double> Qz1d_updated) {
    // this->z_on_knot = z_on_knot;
    matrix<double> tmpQz2d;
    tmpQz2d.set_size((this->m), (this->n));

    for ( int i = 0; i < this->m; i++ ) {
        for ( int j = 0; j < this->n; j++ ) {
            tmpQz2d(i, j) = Qz1d_updated(i * (this->n) + j);
        }
    }

    // this->Qz2d = Qz2d;
    UpdateSpline2dControlPoints(tmpQz2d);
}

void bicubicspline::UpdateSpline2dControlPoints(matrix<double> Qz2d_updated) {
    // boundary condition
    // wxPrintf("enter \n");
    for ( int i = 1; i < this->m + 1; i++ ) {
        this->Qz2d(i, 0)           = Qz2d_updated(i - 1, 0) * 2 - Qz2d_updated(i - 1, 1);
        this->Qz2d(i, this->n + 1) = Qz2d_updated(i - 1, this->n - 1) * 2 - Qz2d_updated(i - 1, this->n - 2);
    }
    // wxPrintf("finish1 \n");
    for ( int j = 1; j < this->n + 1; j++ ) {
        this->Qz2d(0, j)           = Qz2d_updated(0, j - 1) * 2 - Qz2d_updated(1, j - 1);
        this->Qz2d(this->m + 1, j) = Qz2d_updated(this->m - 1, j - 1) * 2 - Qz2d_updated(this->m - 2, j - 1);
    }
    // wxPrintf("finish2 \n");
    //corner
    this->Qz2d(0, 0)                     = Qz2d_updated(0, 0) * 2 - Qz2d_updated(1, 1);
    this->Qz2d(this->m + 1, this->n + 1) = Qz2d_updated(this->m - 1, this->n - 1) * 2 - Qz2d_updated(this->m - 2, this->n - 2);
    this->Qz2d(0, this->n + 1)           = Qz2d_updated(0, this->n - 1) * 2 - Qz2d_updated(1, this->n - 2);
    this->Qz2d(this->m + 1, 0)           = Qz2d_updated(this->m - 1, 0) * 2 - Qz2d_updated(this->m - 2, 1);
    // wxPrintf("finish3 \n");
    for ( int i = 1; i < this->m + 1; i++ ) {
        for ( int j = 1; j < this->n + 1; j++ ) {
            this->Qz2d(i, j) = Qz2d_updated(i - 1, j - 1);
        }
    }

    // this->Qz2d = Qz2d_updated;
}

void bicubicspline::UpdateSplineInterpMapping(matrix<double> x, matrix<double> y) {
    this->x = x;
    this->y = y;
    // this->z                  = z;
    this->Mapping_Mat_Row_no = y.size( );
    this->Mapping_Mat_Col_no = x.size( );
    this->MappingMat         = MappingMatrix(x, y);
}

void bicubicspline::UpdateDiscreteValues(matrix<double> Discret_Values_For_Smooth) {
    // matrix<double>* discrete_values;
    // delete[] this->discrete_values;
    // this->discrete_values = new matrix<double>[this->frame_no];
    this->discrete_values = Discret_Values_For_Smooth;
}

// wx
void bicubicspline::InitializeSplineModelKnotToControl(int row_no, int column_no, float spline_patch_dimy, float spline_patch_dimx, matrix<double> x, matrix<double> y, matrix<double> z_on_knot) {
    this->m                  = row_no;
    this->n                  = column_no;
    this->spline_patch_dim_x = spline_patch_dimx;
    this->spline_patch_dim_y = spline_patch_dimy;
    this->x                  = x;
    this->y                  = y;
    // this->z                  = z;
    this->z_on_knot          = z_on_knot;
    this->total              = (this->m + 2) * (this->n + 2);
    this->totaldim           = (this->m + 2) * (this->n + 2);
    this->Mapping_Mat_Row_no = y.size( );
    this->Mapping_Mat_Col_no = x.size( );
    // long dim = (this->m + 2) * (this->n + 2);
    // this->phi.set_size(totaldim, totaldim);
    CalcPhi( );
    this->Qz2d          = CalcQz(z_on_knot);
    this->MappingMat    = MappingMatrix(x, y);
    this->smooth_interp = SplineSurface(x, y, this->Qz2d);
    // the following method should also work.
    // matrix<double> Qz1d   = reshape(Qz2d, this->total, 1)
    // this->smooth_interp = ApplyMappingMat(Qz1d);
}

void bicubicspline::InitializeSplineModelControl(int row_no, int column_no, float spline_patch_dimy, float spline_patch_dimx, matrix<double> x, matrix<double> y, matrix<double> Qz2d_input) {
    this->m                  = row_no;
    this->n                  = column_no;
    this->spline_patch_dim_x = spline_patch_dimx;
    this->spline_patch_dim_y = spline_patch_dimy;
    this->x                  = x;
    this->y                  = y;
    // this->z                  = z;
    // this->z_on_knot          = z_on_knot;
    this->total              = (this->m + 2) * (this->n + 2);
    this->totaldim           = (this->m + 2) * (this->n + 2);
    this->Mapping_Mat_Row_no = y.size( );
    this->Mapping_Mat_Col_no = x.size( );
    // long dim = (this->m + 2) * (this->n + 2);
    // this->phi.set_size(totaldim, totaldim);
    // CalcPhi( );
    // this->Qz2d            = CalcQz(z_on_knot);
    this->Qz2d          = Qz2d_input;
    this->MappingMat    = MappingMatrix(x, y);
    this->smooth_interp = SplineSurface(x, y, this->Qz2d);
    // the following method should also work.
    // matrix<double> Qz1d   = reshape(Qz2d, this->total, 1)
    // this->smooth_interp = ApplyMappingMat(Qz1d);
}

void bicubicspline::CalcPhi( ) {
    // void InitializeSpline(int row_no, int column_no, float spline_patch_dimy, float spline_patch_dimx) {
    // this->m                  = row_no;
    // this->n                  = column_no;
    // this->spline_patch_dim_x = spline_patch_dimx;
    // this->spline_patch_dim_y = spline_patch_dimy;
    // total                    = (this->m + 2) * (this->n + 2);
    // totaldim                 = (this->m + 2) * (this->n + 2);
    // long dim = (this->m + 2) * (this->n + 2);
    // this->phi.set_size(this->totaldim, this->totaldim);
    // wxPrintf("mark0-------------------------------------------\n");
    this->phi = zeros_matrix<double>(this->totaldim, this->totaldim);
    // wxPrintf("mark1\n");
    for ( int j = 0; j < this->m; j++ ) {
        for ( int i = 0; i < this->n; i++ ) {
            this->phi(i + (j) * this->n, i + (j) * (this->n + 2))         = 1;
            this->phi(i + (j) * this->n, i + (j) * (this->n + 2) + 1)     = 4;
            this->phi(i + (j) * this->n, i + (j) * (this->n + 2) + 2)     = 1;
            this->phi(i + (j) * this->n, i + (j + 1) * (this->n + 2))     = 4;
            this->phi(i + (j) * this->n, i + (j + 1) * (this->n + 2) + 1) = 16;
            this->phi(i + (j) * this->n, i + (j + 1) * (this->n + 2) + 2) = 4;
            this->phi(i + (j) * this->n, i + (j + 2) * (this->n + 2))     = 1;
            this->phi(i + (j) * this->n, i + (j + 2) * (this->n + 2) + 1) = 4;
            this->phi(i + (j) * this->n, i + (j + 2) * (this->n + 2) + 2) = 1;
        }
    }

    //  y- border
    for ( int i = 0; i < this->m; i++ ) {
        this->phi(this->n * this->m + i, (i + 1) * (this->n + 2))     = 1;
        this->phi(this->n * this->m + i, (i + 1) * (this->n + 2) + 1) = -2;
        this->phi(this->n * this->m + i, (i + 1) * (this->n + 2) + 2) = 1;
    }

    //  y+ border
    for ( int i = 0; i < this->m; i++ ) {
        this->phi(this->n * this->m + this->m + i, (i + 1) * (this->n + 2) + (this->n + 2 - 3)) = 1;
        this->phi(this->n * this->m + this->m + i, (i + 1) * (this->n + 2) + (this->n + 2 - 2)) = -2;
        this->phi(this->n * this->m + this->m + i, (i + 1) * (this->n + 2) + (this->n + 2 - 1)) = 1;
    }

    //  x+ border
    for ( int i = 0; i < this->n; i++ ) {
        this->phi(this->n * this->m + 2 * this->m + i, 1 + i)                     = 1;
        this->phi(this->n * this->m + 2 * this->m + i, 1 + i + this->n + 2)       = -2;
        this->phi(this->n * this->m + 2 * this->m + i, 1 + i + 2 * (this->n + 2)) = 1;
    }

    //  x- border
    for ( int i = 0; i < this->n; i++ ) {
        this->phi(this->n * this->m + 2 * this->m + this->n + i, totaldim - 1 - (1 + i))                     = 1;
        this->phi(this->n * this->m + 2 * this->m + this->n + i, totaldim - 1 - (1 + i + (this->n + 2)))     = -2;
        this->phi(this->n * this->m + 2 * this->m + this->n + i, totaldim - 1 - (1 + i + 2 * (this->n + 2))) = 1;
    }
    //  x- y- corner
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n, 0)                     = 1;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n, this->n + 2 + 1)       = -2;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n, 2 * (this->n + 2) + 2) = 1;
    //  x- y+ corner
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 1, (this->n + 2) - 1)                           = 1;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 1, (this->n + 2) + ((this->n + 2) - 1) - 1)     = -2;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 1, 2 * (this->n + 2) + ((this->n + 2) - 2) - 1) = 1;
    //  x+ y- corner
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 2, (this->m + 2) * (this->n + 2) - (this->n + 2))         = 1;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 2, (this->m + 2) * (this->n + 2) - 2 * (this->n + 2) + 1) = -2;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 2, (this->m + 2) * (this->n + 2) - 3 * (this->n + 2) + 2) = 1;
    //  x+ y+ corner
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 3, (this->m + 2) * (this->n + 2) - 2 * (this->n + 2) - 3) = 1;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 3, (this->m + 2) * (this->n + 2) - (this->n + 2) - 2)     = -2;
    this->phi(this->n * this->m + 2 * this->m + 2 * this->n + 3, (this->m + 2) * (this->n + 2) - 1)                     = 1;
    // return phi;
    this->invphi.set_size(this->totaldim, this->totaldim);
    this->invphi = inv(this->phi);
}

matrix<double> bicubicspline::CalcQz(matrix<double> z_on_knot) {
    // void CalcQz(matrix<double> z) {
    matrix<double> Qz1d;
    matrix<double> Qz2d;
    // matrix<double> invphi;
    Qz1d.set_size(this->totaldim, 1);
    Qz2d.set_size((this->m + 2), (this->n + 2));
    // float Pz[total];

    matrix<double> Pz;
    // matrix<double> Qz1d, Qz2d;
    // Pz.set_size(1, total * total);
    Pz.set_size(this->totaldim * this->totaldim, 1);

    for ( int i = 0; i < this->m * this->n; i++ ) {
        Pz(i) = z_on_knot(i);
    }
    for ( int i = this->m * this->n; i < this->total; i++ ) {
        Pz(i) = 0;
    }

    // invphi.set_size(this->totaldim, this->totaldim);
    // invphi = inv(phi);

    Qz1d = this->invphi * Pz * 36;

    for ( int i = 0; i < this->m + 2; i++ ) {
        for ( int j = 0; j < this->n + 2; j++ ) {
            Qz2d(i, j) = Qz1d(i * (this->n + 2) + j);
        }
    }

    return Qz2d;
}

// matrix<double> bicubicspline::CalcQzWithInvPhi(matrix<double> z_on_knot, matrix<double> InvPhi) {
matrix<double> bicubicspline::CalcQzWithInvPhi(matrix<double> InvPhi) {
    // void CalcQz(matrix<double> z) {
    matrix<double> Qz1d;
    matrix<double> Qz2dOutput;
    // matrix<double> invphi;
    Qz1d.set_size(this->totaldim, 1);
    Qz2dOutput.set_size((this->m + 2), (this->n + 2));
    // float Pz[total];
    // wxPrintf("in calc?1\n");
    matrix<double> Pz;
    // matrix<double> Qz1d, Qz2d;
    // Pz.set_size(1, total * total);
    Pz.set_size(this->totaldim * this->totaldim, 1);
    // wxPrintf("in calc2?");
    // wxPrintf("z_on_knot len %d\n", this->z_on_knot.size( ));
    for ( int i = 0; i < this->m * this->n; i++ ) {
        Pz(i) = this->z_on_knot(i);
    }
    for ( int i = this->m * this->n; i < this->total; i++ ) {
        Pz(i) = 0;
    }
    // wxPrintf("in calc?");
    // invphi.set_size(this->totaldim, this->totaldim);
    // invphi = inv(phi);
    // wxPrintf("print Pz\n");
    // for ( int i = 0; i < this->m; i++ ) {
    //     for ( int j = 0; j < this->n; j++ ) {
    //         wxPrintf("%f\t", Pz(i * this->n + j));
    //     }
    //     wxPrintf("\n");
    // }

    Qz1d = InvPhi * Pz * 36;

    for ( int i = 0; i < this->m + 2; i++ ) {
        for ( int j = 0; j < this->n + 2; j++ ) {
            Qz2dOutput(i, j) = Qz1d(i * (this->n + 2) + j);
            // wxPrintf("%f %f\t", Qz2dOutput(i, j), Qz1d(i * (this->n + 2) + j));
        }
        // wxPrintf("\n");
    }
    // wxPrintf("phi");
    // for ( int i = 0; i < this->totaldim; i++ ) {
    //     for ( int j = 0; j < this->totaldim; j++ ) {
    //         wxPrintf(" %f ", this->phi(i, j));
    //     }
    //     wxPrintf("\n");
    // }
    // wxPrintf("invphi");
    // for ( int i = 0; i < this->totaldim; i++ ) {
    //     for ( int j = 0; j < this->totaldim; j++ ) {
    //         wxPrintf(" %f ", InvPhi(i, j));
    //     }
    //     wxPrintf("\n");
    // }
    // wxPrintf("in function knot %f %f \n", Pz(0), Pz(this->n * this->m - 1));
    // wxPrintf("in function knot %f %f \n", z_on_knot(0), z_on_knot(this->n * this->m - 1));
    // wxPrintf("in function invphi %f %f %f\n", InvPhi(0, 0), InvPhi(10, 10), InvPhi(30, 30));
    // wxPrintf(" in function qz %f %f %f\n", Qz2dOutput(0, 0), Qz2dOutput(10, 10), Qz2dOutput(30, 30));

    // this->Qz2d = Qz2d;
    return Qz2dOutput;
}

void bicubicspline::UpdateQz( ) {
    // void CalcQz(matrix<double> z) {
    matrix<double> Qz1d;
    // matrix<double> Qz2dOutput;
    // matrix<double> invphi;
    Qz1d.set_size(this->totaldim, 1);
    // Qz2dOutput.set_size((this->m + 2), (this->n + 2));
    // float Pz[total];
    // wxPrintf("in calc?1\n");
    matrix<double> Pz;
    // matrix<double> Qz1d, Qz2d;
    // Pz.set_size(1, total * total);
    Pz.set_size(this->totaldim * this->totaldim, 1);
    // wxPrintf("in calc2?");
    // wxPrintf("z_on_knot len %d\n", this->z_on_knot.size( ));
    for ( int i = 0; i < this->m * this->n; i++ ) {
        Pz(i) = this->z_on_knot(i);
    }
    for ( int i = this->m * this->n; i < this->total; i++ ) {
        Pz(i) = 0;
    }

    Qz1d = this->invphi * Pz * 36;

    for ( int i = 0; i < this->m + 2; i++ ) {
        for ( int j = 0; j < this->n + 2; j++ ) {
            this->Qz2d(i, j) = Qz1d(i * (this->n + 2) + j);
            // wxPrintf("%f %f\t", this->Qz2d(i, j), Qz1d(i * (this->n + 2) + j));
        }
        // wxPrintf("\n");
    }
    // wxPrintf("before assign \n");
    // this->Qz2d = Qz2dOutput;
    // wxPrintf("in calc?1 done\n");
    // return Qz2dOutput;
}

double bicubicspline::ApplySpline(double u, double v, int pu, int pv, matrix<double> Qz2d) {
    double V1, V2, V3, V4;
    double U1, U2, U3, U4;
    double param_z;

    V1      = powf((1 - v), 3);
    V2      = 3 * powf(v, 3) - 6 * powf(v, 2) + 4;
    V3      = -3 * powf(v, 3) + 3 * powf(v, 2) + 3 * v + 1;
    V4      = powf(v, 3);
    U1      = powf((1 - u), 3);
    U2      = 3 * powf(u, 3) - 6 * powf(u, 2) + 4;
    U3      = -3 * powf(u, 3) + 3 * powf(u, 2) + 3 * u + 1;
    U4      = powf(u, 3);
    param_z = ((V1 * (Qz2d(pu, pv) * U1 + Qz2d(pu + 1, pv) * U2 + Qz2d(pu + 2, pv) * U3 + Qz2d(pu + 3, pv) * U4)) + (V2 * (Qz2d(pu, pv + 1) * U1 + Qz2d(pu + 1, pv + 1) * U2 + Qz2d(pu + 2, pv + 1) * U3 + Qz2d(pu + 3, pv + 1) * U4)) + (V3 * (Qz2d(pu, pv + 2) * U1 + Qz2d(pu + 1, pv + 2) * U2 + Qz2d(pu + 2, pv + 2) * U3 + Qz2d(pu + 3, pv + 2) * U4)) + (V4 * (Qz2d(pu, pv + 3) * U1 + Qz2d(pu + 1, pv + 3) * U2 + Qz2d(pu + 2, pv + 3) * U3 + Qz2d(pu + 3, pv + 3) * U4))) / 36;

    return param_z;
}

matrix<double> bicubicspline::ZOnGrid(matrix<double> Qz2d) {
    matrix<double> GridZ;
    GridZ.set_size(this->m, this->n);
    for ( int i = 0; i < this->m; i++ ) {
        for ( int j = 0; j < this->n; j++ ) {
            GridZ(i, j) = ApplySpline(0.0, 0.0, i, j, Qz2d);
        }
    }
    for ( int i = 0; i < this->m - 1; i++ ) {
        GridZ(i, this->n - 1) = ApplySpline(0.0, 1.0, i, this->n - 2, Qz2d);
    }

    for ( int j = 0; j < this->n - 1; j++ ) {
        GridZ(this->m - 1, j) = ApplySpline(1.0, 0.0, this->m - 2, j, Qz2d);
    }
    GridZ(this->m - 1, this->n - 1) = ApplySpline(1.0, 1.0, this->m - 2, this->n - 2, Qz2d);

    return GridZ;
}

matrix<double> bicubicspline::SplineSurface(matrix<double> x, matrix<double> y, matrix<double> Qz2d) {
    int            lenx = x.size( );
    int            leny = y.size( );
    matrix<double> z_surface;
    // cout << lenx << '\t' << leny << endl;
    z_surface.set_size(leny, lenx);
    // cout << z_surface.size( ) << endl;
    matrix<double> PUY;
    matrix<double> PVX;
    double         u;
    double         v;
    PUY = y / this->spline_patch_dim_y;
    PVX = x / this->spline_patch_dim_x;
    // cout << "PUY" << PUY << endl;
    // cout << "PVX" << PVX << endl;
    if ( PUY(leny - 1) > this->m or PVX(lenx - 1) > this->n ) {
        cout << " the data set exists the boundary of the spline model" << endl;
    }
    // cout << PVX << endl;
    int y_index_start = 0;
    int y_index_end;
    int x_index_start = 0;
    int x_index_end;

    for ( int pv = 0; pv < this->n - 1; pv++ ) {
        // cout << "mark1" << endl;
        if ( x_index_start > lenx - 1 )
            break;
        while ( PVX(x_index_start) < pv )
            x_index_start++;
        x_index_end = x_index_start;

        if ( PVX(x_index_start) >= (pv + 1) )
            continue;

        while ( PVX(x_index_end) < (pv + 1) && x_index_end <= lenx - 1 ) {
            x_index_end++;
        }
        if ( x_index_end > x_index_start )
            x_index_end -= 1;
        // x_index_end -= 1;
        // cout << "mark2" << endl;
        // cout << "xindex " << x_index_start << " " << x_index_end << endl;
        y_index_start = 0;
        for ( int pu = 0; pu < this->m - 1; pu++ ) {
            if ( y_index_start > leny - 1 )
                break;
            while ( PUY(y_index_start) < pu )
                y_index_start++;
            if ( PUY(y_index_start) >= (pu + 1) )
                continue;
            y_index_end = y_index_start;
            while ( PUY(y_index_end) < (pu + 1) && y_index_end <= leny - 1 ) {
                y_index_end++;
            }
            if ( y_index_end > y_index_start )
                y_index_end -= 1;
            // cout << "mark3" << endl;
            // cout << "yindex " << y_index_start << " " << y_index_end << endl;
            for ( int i = y_index_start; i <= y_index_end; i++ ) {
                for ( int j = x_index_start; j <= x_index_end; j++ ) {
                    u = PUY(i) - pu;
                    v = PVX(j) - pv;
                    // cout << "mark31" << endl;
                    // cout << i << " " << j << " " << PUY(i) << " " << PVX(j) << " " << u << " " << v << endl;
                    z_surface(i, j) = ApplySpline(u, v, pu, pv, Qz2d);
                    // cout << "mark32" << endl;
                }
            }
            // y_index_start = 0;
            // y_index_end   = y_index_start;
            y_index_start = y_index_end + 1;
        }
        x_index_start = x_index_end + 1;
        // x_index_end   = x_index_start;
    }
    // PVX(lenx-1) should be smaller than n-1
    // if ( (x_index_start <= (lenx - 1)) && (PVX(x_index_start) - (n - 1)) == 0 )
    // cout << "mark4" << endl;
    if ( (x_index_start <= (lenx - 1)) && ((PVX(x_index_start) - (this->n - 1)) == 0) ) {
        int pv        = this->n - 2;
        y_index_start = 0;
        // cout << "mark5" << endl;
        for ( int pu = 0; pu < this->m - 1; pu++ ) {
            while ( PUY(y_index_start) < pu )
                y_index_start++;
            if ( PUY(y_index_start) >= (pu + 1) )
                continue;
            y_index_end = y_index_start;
            while ( PUY(y_index_end) < (pu + 1) && y_index_end <= leny - 1 ) {
                y_index_end++;
            }
            if ( y_index_end > y_index_start )
                y_index_end -= 1;
            for ( int i = y_index_start; i <= y_index_end; i++ ) {
                // u = PUY(i) - pu;
                // v = 0;
                u = PUY(i) - pu;
                v = PVX(x_index_start) - pv;
                // cout << i << " " << x_index_start << " " << u << " " << v << endl;

                z_surface(i, x_index_start) = ApplySpline(u, v, pu, pv, Qz2d);
            }
            y_index_start = y_index_end + 1;
            // y_index_start = 0;
        }

        if ( (y_index_start <= (leny - 1)) && ((PUY(y_index_start) - (this->m - 1)) == 0) ) {
            int pu = this->m - 2;
            // u      = 0;
            // v      = 0;
            u = PUY(y_index_start) - pu;
            v = PVX(x_index_start) - pv;
            // cout << y_index_start << " " << x_index_start << " " << PUY(y_index_start) << " " << PVX(x_index_start) << " " << u << " " << v << endl;
            z_surface(y_index_start, x_index_start) = ApplySpline(u, v, pu, pv, Qz2d);
        }
    }
    // cout << "mark6" << endl;
    if ( (y_index_start <= (leny - 1)) && ((PUY(y_index_start) - (this->m - 1)) == 0) ) {
        int pu        = this->m - 2;
        x_index_start = 0;
        for ( int pv = 0; pv < this->n - 1; pv++ ) {
            while ( PVX(x_index_start) < pv )
                x_index_start++;
            if ( PVX(x_index_start) >= (pv + 1) )
                continue;
            x_index_end = x_index_start;

            while ( PVX(x_index_end) < (pv + 1) && x_index_end <= lenx - 1 ) {
                x_index_end++;
            }
            if ( x_index_end > x_index_start )
                x_index_end -= 1;
            for ( int j = x_index_start; j <= x_index_end; j++ ) {
                u = PUY(y_index_start) - pu;
                v = PVX(x_index_start) - pv;
                // cout << y_index_start << " " << j << " " << PUY(y_index_start) << " " << PVX(j) << " " << u << " " << v << endl;
                z_surface(y_index_start, j) = ApplySpline(u, v, pu, pv, Qz2d);
            }
            x_index_start = x_index_end + 1;
        }
    }
    // cout << "mark7" << endl;
    // for ( int i = 0; i < leny; i++ ) {
    //     for ( int j = 0; j < lenx; j++ ) {
    //         // cout << i << " " << j << " " << z_surface(i, j) << "  ";
    //         cout << z_surface(i, j) << "  ";
    //     }
    //     cout << endl;
    // }
    return z_surface;
}

matrix<double> bicubicspline::MappingMatrix(matrix<double> x, matrix<double> y) {
    int lenx = x.size( );
    int leny = y.size( );
    // matrix<double> z_surface;
    matrix<double> MappingMat;
    // cout << lenx << '\t' << leny << endl;
    // z_surface.set_size(leny, lenx);
    MappingMat.set_size(leny * lenx, 6);
    int count = 0;
    // cout << z_surface.size( ) << endl;
    matrix<double> PUY;
    matrix<double> PVX;
    double         u;
    double         v;
    PUY                      = y / this->spline_patch_dim_y;
    PVX                      = x / this->spline_patch_dim_x;
    this->Mapping_Mat_Row_no = leny;
    this->Mapping_Mat_Col_no = lenx;

    // cout << "PUY" << PUY << endl;
    // cout << "PVX" << PVX << endl;
    if ( PUY(leny - 1) > this->m or PVX(lenx - 1) > this->n ) {
        cout << " the data set exists the boundary of the spline model" << endl;
    }
    // cout << PVX << endl;
    int y_index_start = 0;
    int y_index_end;
    int x_index_start = 0;
    int x_index_end;

    for ( int pv = 0; pv < this->n - 1; pv++ ) {
        // cout << "mark1" << endl;
        if ( x_index_start > lenx - 1 )
            break;
        while ( PVX(x_index_start) < pv )
            x_index_start++;
        x_index_end = x_index_start;

        if ( PVX(x_index_start) >= (pv + 1) )
            continue;

        while ( PVX(x_index_end) < (pv + 1) && x_index_end <= lenx - 1 ) {
            x_index_end++;
        }
        if ( x_index_end > x_index_start )
            x_index_end -= 1;
        // x_index_end -= 1;
        // cout << "mark2" << endl;
        // cout << "xindex " << x_index_start << " " << x_index_end << endl;
        // set_subm(MappingMat, range(0, 1), range(0, 1)) = 7;
        y_index_start = 0;
        for ( int pu = 0; pu < this->m - 1; pu++ ) {
            if ( y_index_start > leny - 1 )
                break;
            while ( PUY(y_index_start) < pu )
                y_index_start++;
            if ( PUY(y_index_start) >= (pu + 1) )
                continue;
            y_index_end = y_index_start;
            while ( PUY(y_index_end) < (pu + 1) && y_index_end <= leny - 1 ) {
                y_index_end++;
            }
            if ( y_index_end > y_index_start )
                y_index_end -= 1;
            // cout << "mark3" << endl;
            // cout << "yindex " << y_index_start << " " << y_index_end << endl;
            for ( int i = y_index_start; i <= y_index_end; i++ ) {
                for ( int j = x_index_start; j <= x_index_end; j++ ) {
                    u = PUY(i) - pu;
                    v = PVX(j) - pv;
                    // cout << "mark31" << endl;
                    // cout << i << " " << j << " " << PUY(i) << " " << PVX(j) << " " << u << " " << v << endl;
                    MappingMat(count, 0) = i;
                    MappingMat(count, 1) = j;
                    MappingMat(count, 2) = pu;
                    MappingMat(count, 3) = pv;
                    MappingMat(count, 4) = u;
                    MappingMat(count, 5) = v;
                    count++;
                    // z_surface(i, j) = ApplySpline(u, v, pu, pv, Qz2d);
                    // cout << "mark32" << endl;
                }
            }
            // y_index_start = 0;
            // y_index_end   = y_index_start;
            y_index_start = y_index_end + 1;
        }
        x_index_start = x_index_end + 1;
        // x_index_end   = x_index_start;
    }
    // PVX(lenx-1) should be smaller than n-1
    // if ( (x_index_start <= (lenx - 1)) && (PVX(x_index_start) - (n - 1)) == 0 )
    // cout << "mark4" << endl;
    if ( (x_index_start <= (lenx - 1)) && ((PVX(x_index_start) - (this->n - 1)) == 0) ) {
        int pv        = this->n - 2;
        y_index_start = 0;
        // cout << "mark5" << endl;
        for ( int pu = 0; pu < this->m - 1; pu++ ) {
            while ( PUY(y_index_start) < pu )
                y_index_start++;
            if ( PUY(y_index_start) >= (pu + 1) )
                continue;
            y_index_end = y_index_start;
            while ( PUY(y_index_end) < (pu + 1) && y_index_end <= leny - 1 ) {
                y_index_end++;
            }
            if ( y_index_end > y_index_start )
                y_index_end -= 1;
            for ( int i = y_index_start; i <= y_index_end; i++ ) {
                // u = PUY(i) - pu;
                // v = 0;
                u = PUY(i) - pu;
                v = PVX(x_index_start) - pv;
                // cout << i << " " << x_index_start << " " << u << " " << v << endl;
                MappingMat(count, 0) = i;
                MappingMat(count, 1) = x_index_start;
                MappingMat(count, 2) = pu;
                MappingMat(count, 3) = pv;
                MappingMat(count, 4) = u;
                MappingMat(count, 5) = v;
                count++;
                // z_surface(i, x_index_start) = ApplySpline(u, v, pu, pv, Qz2d);
            }
            y_index_start = y_index_end + 1;
            // y_index_start = 0;
        }

        if ( (y_index_start <= (leny - 1)) && ((PUY(y_index_start) - (this->m - 1)) == 0) ) {
            int pu               = this->m - 2;
            u                    = PUY(y_index_start) - pu;
            v                    = PVX(x_index_start) - pv;
            MappingMat(count, 0) = y_index_start;
            MappingMat(count, 1) = x_index_start;
            MappingMat(count, 2) = pu;
            MappingMat(count, 3) = pv;
            MappingMat(count, 4) = u;
            MappingMat(count, 5) = v;
            count++;
            // z_surface(y_index_start, x_index_start) = ApplySpline(u, v, pu, pv, Qz2d);
        }
    }

    // cout << "mark6" << endl;
    if ( (y_index_start <= (leny - 1)) && ((PUY(y_index_start) - (this->m - 1)) == 0) ) {
        int pu        = this->m - 2;
        x_index_start = 0;
        for ( int pv = 0; pv < this->n - 1; pv++ ) {
            while ( PVX(x_index_start) < pv )
                x_index_start++;
            if ( PVX(x_index_start) >= (pv + 1) )
                continue;
            x_index_end = x_index_start;

            while ( PVX(x_index_end) < (pv + 1) && x_index_end <= lenx - 1 ) {
                x_index_end++;
            }
            if ( x_index_end > x_index_start )
                x_index_end -= 1;
            for ( int j = x_index_start; j <= x_index_end; j++ ) {
                u                    = PUY(y_index_start) - pu;
                v                    = PVX(x_index_start) - pv;
                MappingMat(count, 0) = y_index_start;
                MappingMat(count, 1) = j;
                MappingMat(count, 2) = pu;
                MappingMat(count, 3) = pv;
                MappingMat(count, 4) = u;
                MappingMat(count, 5) = v;
                count++;
                // z_surface(y_index_start, j) = ApplySpline(u, v, pu, pv, Qz2d);
            }
            x_index_start = x_index_end + 1;
        }
    }

    return MappingMat;
}

// matrix<double> ImplementParam(bicubicspline& splinemodel, matrix<double> Qz2d) {
//     // for (int i=0;i<)
// }

// matrix<double> bicubicspline::ApplyMappingMat(matrix<double> Qz1d) {
// Qz2d = reshape(Qz1d, (this->m + 2), (this->n + 2));
matrix<double> bicubicspline::ApplyMappingMat(matrix<double> Qz2d) {
    // Qz2d = reshape(Qz1d, (this->m + 2), (this->n + 2));
    // wxPrintf("in smooth apply?\n");
    // wxPrintf(" mappin mat row and col %f %f\n", this->Mapping_Mat_Row_no, this->Mapping_Mat_Col_no);
    // wxPrintf(" mappin mat row and col %d %d\n", int(this->Mapping_Mat_Row_no), int(this->Mapping_Mat_Col_no));
    matrix<double> smoothsurf;
    smoothsurf.set_size(this->Mapping_Mat_Row_no, this->Mapping_Mat_Col_no);
    // wxPrintf("in smooth apply 1?\n");
    int   total_no = this->Mapping_Mat_Col_no * this->Mapping_Mat_Row_no;
    int   row_no   = this->Mapping_Mat_Row_no;
    int   col_no   = this->Mapping_Mat_Col_no;
    int   pu;
    int   pv;
    float u;
    float v;
    int   col;
    int   row;
    // wxPrintf("smooth apply?\n");
    for ( int i = 0; i < total_no; i++ ) {
        row                  = this->MappingMat(i, 0);
        col                  = this->MappingMat(i, 1);
        pu                   = this->MappingMat(i, 2);
        pv                   = this->MappingMat(i, 3);
        u                    = this->MappingMat(i, 4);
        v                    = this->MappingMat(i, 5);
        smoothsurf(row, col) = ApplySpline(u, v, pu, pv, Qz2d);
    }
    this->smooth_interp = smoothsurf;
    return smoothsurf;
}

double bicubicspline::ApplySplineFunc(double xp, double yp) {
    double zp;
    double PUY;
    double PVX;
    double u;
    double v;
    PUY = yp / this->spline_patch_dim_y;
    PVX = xp / this->spline_patch_dim_x;
    int pv;
    int pu;

    pv = int(PVX);
    pu = int(PUY);

    // cout << " here "
    //      << "PUY PVX pu pv " << PUY << " " << PVX << " " << pu << " " << pv << endl;

    // if ( ((PVX - (this->n - 1)) == 0) ) {
    if ( (int(PVX) == (this->n - 1)) && (PVX <= this->n - 1) ) {
        pv = this->n - 2;
    }

    if ( (int(PUY) == (this->m - 1)) && (PUY <= this->m - 1) ) {
        pu = this->m - 2;
    }

    u = PUY - pu;
    v = PVX - pv;

    if ( PUY > this->m - 1 ) {
        // cout << "spline dis x y" << this->spline_patch_dim_x << " " << this->spline_patch_dim_y << endl;
        // cout << " xp " << xp << endl;
        // cout << " yp " << yp << endl;
        // cout << "PUY " << PUY << " m-1 " << this->m - 1 << endl;
        // cout << "u " << u << endl;
        // cout << "the y position of the point exceed the spline surface " << endl;
        // cout << "replaced with the boundary value\n " << endl;
        // use the boundary value as the value outside the spline area. This need to be revised. Or the program should be forced to terminate.
        pu = this->m - 2;
        u  = 1;
    }
    if ( PVX > this->n - 1 ) {
        // cout << "spline dis x y" << this->spline_patch_dim_x << " " << this->spline_patch_dim_y << endl;
        // cout << " xp " << xp << endl;
        // cout << " yp " << yp << endl;
        // cout << "PVX " << PVX << " n-1 " << this->n - 1 << endl;
        // cout << "v " << v << endl;
        // cout << "the x position of the point exceed the spline surface " << endl;
        // cout << "replaced with the boundary value\n " << endl;

        pv = this->n - 2;
        v  = 1;
    }

    // cout << "u v pu pv " << u << " " << v << " " << pu << " " << pv << endl;
    // wxPrintf("PUY,PVX,u,v,pu,pv, %f %f %f %f %d %d \n", PUY, PVX, u, v, pu, pv);
    zp = ApplySpline(u, v, pu, pv, this->Qz2d);
    // cout << "zp " << zp << endl;

    return zp;
}

// double MissingGridFixObject(matrix<double> z_on_knot_refined) {
//     matrix<double> Qz1d;
//     matrix<double> Qz2d_refined;
//     matrix<double> smoothsurf;
//     matrix<double> updated_z_on_knot;
//     double         error = 0;

//     Qz2d_refined           = CalcQz(z_on_knot_refined);
//     updated_z_on_knot      = reshape(ZOnGrid(Qz2d_refined), totaldim, 1);
//     int UsedIndex_total_no = this->UsedIndex.size( );

//     for ( int i = 0; i < UsedIndex_total_no; i++ ) {
//         int ind = this->UsedIndex[i];
//         error   = error + powf(updated_z_on_knot(ind) - this->z_on_knot(ind), 2);
//     }

//     return error;
// }

// double MissingGridFixObject(matrix<double> Qz1d) {

//     // matrix<double> Qz1d;
//     matrix<double> Qz2d_refined;
//     matrix<double> smoothsurf;
//     matrix<double> updated_z_on_knot;
//     double         error = 0;
//     Qz2d_refined         = reshape(Qz1d, (this->m + 2), (this->n + 2));
//     // Qz2d_refined           = CalcQz(z_on_knot_refined);
//     updated_z_on_knot      = reshape(ZOnGrid(Qz2d_refined), totaldim, 1);
//     int UsedIndex_total_no = this->UsedIndex.size( );

//     for ( int i = 0; i < UsedIndex_total_no; i++ ) {
//         int ind = this->UsedIndex[i];
//         error   = error + powf(updated_z_on_knot(ind) - this->z_on_knot(ind), 2);
//     }

//     return error;
// }

double bicubicspline::OptimizationObejct(matrix<double> Qz1d) {

    // matrix<double> Qz2d = reshape(Qz1d, (this->m + 2), (this->n + 2));
    matrix<double> smoothsurf;
    // smoothsurf.set_size(this->Mapping_Mat_Row_no, this->Mapping_Mat_Col_no);
    int total_no = this->Mapping_Mat_Col_no * this->Mapping_Mat_Row_no;
    int row_no   = this->Mapping_Mat_Row_no;
    int col_no   = this->Mapping_Mat_Col_no;
    // wxPrintf("enter optimize obejct\n");
    double error;
    // cout << "total number" << total_no << endl;
    UpdateSpline1dControlPoints(Qz1d);
    smoothsurf = ApplyMappingMat(this->Qz2d);

    error = 0.0;
    for ( int i = 0; i < row_no; i++ ) {
        for ( int j = 0; j < col_no; j++ ) {
            // error = error + std::abs(smoothsurf(i, j) - this->z(i * col_no + j));
            error = error + std::abs(smoothsurf(i, j) - this->discrete_values(i, j));
            // cout << i * col_no + j << " " << error << " " << smoothsurf(i, j) << " " << z(i * col_no + j) << endl;
        }
    }
    return error;
}

// double bicubicspline::OptimizationKnotObejct(matrix<double> join1d) {
//     matrix<double> Join2d = reshape(join1d, (this->m), (this->n));
//     matrix<double> smoothsurf;
//     // this->z_on_knot = Join2d;
//     matrix<double> Qz2d;
//     Qz2d = this->CalcQz(Join2d);
//     smoothsurf.set_size(this->Mapping_Mat_Row_no, this->Mapping_Mat_Col_no);
//     int    total_no = this->Mapping_Mat_Col_no * this->Mapping_Mat_Row_no;
//     int    row_no   = this->Mapping_Mat_Row_no;
//     int    col_no   = this->Mapping_Mat_Col_no;
//     int    pu;
//     int    pv;
//     float  u;
//     float  v;
//     int    col;
//     int    row;
//     double error;
//     // cout << "total number" << total_no << endl;

//     for ( int i = 0; i < total_no; i++ ) {
//         col                  = this->MappingMat(i, 0);
//         row                  = this->MappingMat(i, 1);
//         pu                   = this->MappingMat(i, 2);
//         pv                   = this->MappingMat(i, 3);
//         u                    = this->MappingMat(i, 4);
//         v                    = this->MappingMat(i, 5);
//         smoothsurf(col, row) = ApplySpline(u, v, pu, pv, Qz2d);
//     }

//     error = 0.0;
//     for ( int i = 0; i < row_no; i++ ) {
//         for ( int j = 0; j < col_no; j++ ) {
//             error = error + std::abs(smoothsurf(i, j) - this->z(i * col_no + j));
//             // cout << i * col_no + j << " " << error << " " << smoothsurf(i, j) << " " << z(i * col_no + j) << endl;
//         }
//     }

//     return error;
// }

// void Search( ) {
//     matrix<double> Qz1dtest = zeros_matrix<double>(long(m + 2) * long(n + 2), 1);
//     // Qz1dtest=reshape()
//     // Qz1dtest = zeros_matrix(7, 10);

//     // cout << spline.OptimizationObejct(qz1d) << endl;
//     // matrix         starting_point = {4, 8};
//     // column<double> starting_point = {-94, 5.2};
//     find_min_using_approximate_derivatives(lbfgs_search_strategy(10),
//                                            objective_delta_stop_strategy(1e-7),
//                                            OptimizationObejct, Qz1dtest, -1);
//     cout << "solution: \n"
//          << Qz1dtest << endl;
// }
// };