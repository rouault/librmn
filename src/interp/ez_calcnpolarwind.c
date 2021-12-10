/* RMNLIB - Library of useful routines for C and FORTRAN programming
 * Copyright (C) 1975-2001  Division de Recherche en Prevision Numerique
 *                          Environnement Canada
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation,
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include "ezscint.h"
#include "ez_funcdef.h"

int32_t ez_calcnpolarwind(
    float *polar_uu_in,
    float *polar_vv_in,
    float *uuin,
    float *vvin,
    int32_t ni,
    int32_t nj,
    int32_t gdin
) {
    int32_t k1, k2;
    float *polar_wd, *polar_spd,*polar_lat,*polar_lon,*polar_lat_gem, *polar_lon_gem, *polar_x, *polar_y, *polar_uu, *polar_vv;
    char  grtyp[2],grref[2],grtypn[2],grtypa[2];
    int32_t ig1in,ig2in,ig3in,ig4in,ig1in_ref,ig2in_ref,ig3in_ref,ig4in_ref;
    float xlat1, xlat2, xlon1, xlon2;
    int32_t ig1n, ig2n, ig3n, ig4n;
    float pi, pj, d60, dgrw;
    int32_t  ier, gdps, gda, gdrow, gdcol;
    float uupole, vvpole;
    float quatrevingtdix, zero;


    c_gdkey2rowcol(gdin, &gdrow, &gdcol);
    polar_uu  = (float *) malloc(ni*sizeof(float));
    polar_vv  = (float *) malloc(ni*sizeof(float));
    polar_wd  = (float *) malloc(ni*sizeof(float));
    polar_spd = (float *) malloc(ni*sizeof(float));
    polar_lat = (float *) malloc(ni*sizeof(float));
    polar_lon = (float *) malloc(ni*sizeof(float));
    polar_x   = (float *) malloc(ni*sizeof(float));
    polar_y   = (float *) malloc(ni*sizeof(float));

    for (int32_t i = 0; i < ni; i++) {
        polar_x[i] = 1.0 * (i+1);
        polar_y[i] = 1.0 * nj;
    }

    c_gdllfxy_orig(gdin, polar_lat, polar_lon, polar_x, polar_y, ni);
    ier = c_ezgxprm(gdin, &ni, &nj, grtyp, &ig1in, &ig2in, &ig3in, &ig4in,
            grref, &ig1in_ref, &ig2in_ref, &ig3in_ref, &ig4in_ref);

    if (grtyp[0] == 'Z' && grref[0] == 'E') {
        polar_lat_gem   = (float *) malloc(ni*sizeof(float));
        polar_lon_gem   = (float *) malloc(ni*sizeof(float));

        for (int32_t i = 0; i < ni; i++) {
            polar_lat_gem[i] = polar_lat[i];
            polar_lon_gem[i] = polar_lon[i];
        }

        f77name(cigaxg)(grref, &xlat1, &xlon1, &xlat2, &xlon2, &ig1in_ref, &ig2in_ref, &ig3in_ref, &ig4in_ref,1);
        f77name(ez_gfxyfll)(polar_lon_gem, polar_lat_gem, polar_lon, polar_lat, &ni, &xlat1, &xlon1, &xlat2, &xlon2);
    }

    grtypa[0] = 'A';
    gda = c_ezqkdef(24,12, grtypa, 0,0,0,0,0);
    c_gdwdfuv(gda, polar_spd, polar_wd,  &uuin[(nj-1)*ni], &vvin[(nj-1)*ni], polar_lat, polar_lon, ni);

    pi   = 0.0;
    pj   = 0.0;
    d60  = 1000.0;
    dgrw = 0.0;
    grtypn[0] = 'N';
    f77name(cxgaig)(grtypn, &ig1n, &ig2n, &ig3n, &ig4n, &pi, &pj, &d60, &dgrw,1);
    gdps = c_ezqkdef(ni, 1, grtypn, ig1n, ig2n, ig3n, ig4n, 0);
    c_gduvfwd(gdps, polar_uu, polar_vv, polar_spd,  polar_wd, polar_lat, polar_lon, ni);

    f77name(ez_calcpoleval)(&uupole, polar_uu, &ni, Grille[gdrow][gdcol].ax, &Grille[gdrow][gdcol].grtyp, &Grille[gdrow][gdcol].grref,1,1);
    f77name(ez_calcpoleval)(&vvpole, polar_vv, &ni, Grille[gdrow][gdcol].ax, &Grille[gdrow][gdcol].grtyp, &Grille[gdrow][gdcol].grref,1,1);

    quatrevingtdix = 90.0;
    zero = 0.0;
    c_gdwdfuv(gdps, polar_spd, polar_wd,  &uupole, &vvpole, &quatrevingtdix, &zero, 1);


    polar_lat[0] = 90.0;
    for (int32_t i = 1; i < ni; i++) {
        polar_wd[i]  = polar_wd[0] + polar_lon[i];
        polar_spd[i] = polar_spd[0];
        polar_lat[i] = 90.0;
    }
    polar_wd[0] = polar_wd[0] + polar_lon[0];

    c_gduvfwd(gda, polar_uu, polar_vv, polar_spd,  polar_wd, polar_lat, polar_lon, ni);

    for (int32_t j = 0; j < 3; j++) {
        for (int32_t i = 0; i < ni; i++) {
            k1 = j * ni + i;
            k2 = (nj-3+j)  * ni + i;
            polar_uu_in[k1] = uuin[k2];
            polar_vv_in[k1] = vvin[k2];
        }
    }

    for (int32_t i = 0; i < ni; i++) {
        k1 = 3 * ni+i;
        polar_uu_in[k1] = polar_uu[i];
        polar_vv_in[k1] = polar_vv[i];
    }

    free(polar_y);
    free(polar_x);
    free(polar_lat);
    free(polar_lon);
    free(polar_spd);
    free(polar_wd);
    free(polar_vv);
    free(polar_uu);

    //  ier = c_gdrls(gdps);
    return 0;
}
