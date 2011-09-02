#encoding: utf-8
import imp, os, sys
import numpy as np


import scipy.weave
from scipy.weave import converters

def main_is_frozen():
    return (hasattr(sys, "frozen") or # new py2exe
            hasattr(sys, "importers") # old py2exe
            or imp.is_frozen("__main__")) # tools/freeze
 
def build_extension():

    import scipy.weave.blitz_spec
    # scipy.weave thinks that visual c++ does not work, it works allright for our code.
    # patch it so that it does not throw an error.
    scipy.weave.blitz_spec.array_info.check_compiler = (lambda self, compiler: None)
    print "patched the check_compiler method"

    from scipy.weave import ext_tools
    mod = ext_tools.ext_module('bildregistrierung_ng_ext')
    




# correlation of two arrays
# arguments:
#   I,T:  2-dimensional ndarrays of np.float
#   returns: cross-correlation of the two arrays
    I = np.ndarray((20,20),np.float)
    T = np.ndarray((20,20),np.float)

    code = """ 
    double I_mean = mean(I);
    double T_mean = mean(T);
    return_val = mean((I-I_mean) * (T-T_mean)) / (sqrt(mean(pow2(I-I_mean))) * sqrt(mean(pow2(T-T_mean))));
    """


    func = ext_tools.ext_function('correlation', code, ["I","T"] ,type_converters=converters.blitz)
    mod.add_function(func)


# transform data
# arguments:
#   data: ndarray to transform
#   dx, dy, alpha, scale: the transformation
#   result: where the transformed data should be stored.
    alpha = scale = 1.0
    transformation = np.ndarray((4,), np.float) # dx, dy, alpha, scale
    dx = dy = 0.0
    top = left = 0
    data = result = np.ndarray((1,1), np.float)
    code = r"""
    # line 10047 "bildregistrierung-ng.py"
    // transform backwards, ->   / scale
    double dx = transformation(0);
    double dy = transformation(1);
    double alpha = transformation(2);
    double scale = transformation(3);

    float c = cos(alpha);
    float s = sin(alpha);

    int w  = data.extent(1);
    int h  = data.extent(0);
    int rw = result.extent(1);
    int rh = result.extent(0);

    for (int ry = 0; ry < rh; ry++){
        for(int rx = 0; rx < rw; rx++){

            int x = rx + left;
            int y = ry + top;

            // transform backwards
            int tx = (int) w/2 + c * (x-w/2) + s * (y-h/2) - dx;
            int ty = (int) h/2 - s * (x-w/2) + c * (y-h/2) - dy;
            result(ry, rx) = (tx >= 0 && tx < w && ty >= 0 && ty < h ?
                            data(ty, tx): -1);
        }
    }
    """
    func = ext_tools.ext_function('transform_part', code, ["data","transformation","result", "left", "top"] ,
                                    type_converters=converters.blitz)
    func.customize.add_support_code("#include <cmath> \n")
    mod.add_function(func)

# find best displacement for a given rotation and scale value.
#    ("x","y","w","h","a_kwerte","maxdelta","alpha","scale", "result")
# x, y: center of the areas for which correlation values were calculated
# w, h: width and height of the images to fit on each other
# kwerte: the colleration values for displacements of the areas belonging to the points x,y.
#         3-dimensional, the first dimension is for which x,y point, the other two
#         dimensions are the displacement in x and y direction
# maxdelta: the maximum displacement in x- and y- direction for which colleration values have been calculated
# alpha: the angle of rotation of the transformation
# scale: the scaling of the transformation
# result: (best_kvalue, dx, dy, alpha, scale): if a result with a better k-value is found, it is stored into this array.
    w = h = 1
    xr = yr = np.ndarray(1,np.int32) # x and y values of the points relative to the centre
    dx = dy = 0.0
    kwerte = np.ndarray((1,1,1),np.float)
    maxdelta  = 1
    alpha = scale = 1.0
    result = np.ndarray((1),np.float)
    translation_stepsize = 1.0


    code = r"""
    #line 100100 "bildregistrierung-ng.py"
    using namespace blitz;

    float c = cos(alpha) * scale;
    float s = sin(alpha) * scale;

    // verschiebung der einzelnen Punkte durch rotation um das zentrum
    Array<double,1> rdx ((c-1) * xr  - s     * yr);
    Array<double,1> rdy (s     * xr  + (c-1) * yr);

    // maximale verschiebung in beide richtungen

    double max_rot_delta_x = max(abs(rdx));
    double max_rot_delta_y = max(abs(rdy));

    const int kwerte_size = kwerte.extent(1);

    double xstart = max(-kwerte_size/2 + max_rot_delta_x + 1, dx - maxdelta);
    double ystart = max(-kwerte_size/2 + max_rot_delta_y + 1, dy - maxdelta);
    double xend   = min(+kwerte_size/2 - max_rot_delta_x - 1, dx + maxdelta);
    double yend   = min(+kwerte_size/2 - max_rot_delta_x - 1, dx + maxdelta);

    //printf("minrdx = %f maxrdx = %f minrdy = %f maxrdy = %f, x: %f:%f y: %f:%f\n",
    //       (float)minrdx, (float) maxrdx, (float) minrdy, (float) maxrdy, 
    //       (float) xstart, (float)xend, (float)ystart, (float)yend);
    //printf("x: %lf:%lf, y: %lf:%lf\n", xstart, xend, ystart, yend);

    for (double dx = xstart; dx < xend; dx+= translation_stepsize){
        for (double dy = ystart; dy < yend; dy+= translation_stepsize){
            double tmp = 1;


            int size = kwerte.extent(1);
            for(int i = 0; i < rdx.extent(0); i++){
                //           rotation  verschiebung  runden  relativ zur mitte der kwerte
                int xi = (int) (rdx(i) + dx          +  0.5 + kwerte.extent(1)/2);
                int yi = (int) (rdy(i) + dy          +  0.5 + kwerte.extent(2)/2);
                if (xi < 0 || xi >= size || xi < 0 || yi >= size){
                    //printf("dx, dy = %lf %lf\n",dx, dy);
                }
                tmp *= kwerte(i, yi, xi);
            }

            if (tmp > result(0)){
                result = tmp, dx, dy, alpha, scale;
                //printf("---- %lf %lf %i %i\n",(double) tmp, (double)result(0), (int)dx, (int)dy);
            }
        }
    }

    """

    func = ext_tools.ext_function('find_best_for_angle', code, ["xr","yr","dx", "dy","kwerte","maxdelta","alpha","scale", "translation_stepsize","result"] ,
                                    #headers=["<stdio.h>"],
                                    type_converters=converters.blitz)
    mod.add_function(func)


    mod.compile(verbose=2)
    #mod.compile(compiler="msvc", verbose=2)


if not main_is_frozen():
    build_extension()

from  bildregistrierung_ng_ext import transform_part, correlation

def transform(data, transformation, result):
    assert(data.shape == result.shape)
    transform_part(data, transformation, result, 0,0)








def korrelationswert(I,T,x,y):
    h,w = T.shape
    return correlation(I[y:y+h, x:x+w],T)

def korrelationswerte(I,T):
    I = np.copy(I)
    T = np.copy(T)
    # maximale Verschiebung von I, dass es immer noch in T ist
    max_x = I.shape[1] - T.shape[1]
    max_y = I.shape[0] - T.shape[0]

    kwerte = np.zeros((max_y, max_x), np.float)

    for y in range(max_y):
        for x in range(max_x):
            kwerte[y,x] = korrelationswert(I,T,x,y)

    #im = plt.imshow(kwerte, interpolation='bilinear', cmap=cm.gray,
    #                origin='lower', extent=[-3,3,-3,3])

    return kwerte

def kwerte_position(I,T, x, y, groesse, delta, counter = [1]):
    counter[0]+= 1

    tI = I[y-delta-groesse:y+delta+groesse, x-delta-groesse:x+delta+groesse]
    tT = T[y-groesse:y+groesse, x-groesse:x+groesse]
    return korrelationswerte(tI, tT)






def default_callback(value):
    print "*" * int(value*100) + "-" * (100 - int(value*100))
def transformation_berechnen(I, T,    korrelationsgroesse = 32, maxdelta = 128, border = 256, N = 8, callback = default_callback):
    """ 
    calculates the best transformation to place T over I
    return: dx, dy, alpha, scale
    dx: displacement in x-direction
    dy: displacement in y-direction
    alpha: rotation around the center, bevor displacement
    scale: scalling around center, bevor displacement
    """
    count_kwerte = 0
    count_transformations = 0


    h, w = I.shape

# rotation: +- delta_alpha
    alpha_max  =   np.arcsin(korrelationsgroesse * 1.0 / (h-border*2))
    alpha_step_1px =  np.arcsin(1.0 / (h-border*2))  # how much alpha has to change to translate approximately a pixel at the edge

    print "alpha_max = %f, alpha_step = %f"%(alpha_max, alpha_step_1px)


    
# the positions for which the k-values are calculated
    x = np.arange(N) * (w-border*2)/(N-1) + border
    y = np.arange(N) * (h-border*2)/(N-1) + border
    xr = x - w/2
    yr = y - h/2

    print "xr, yr",xr,yr



    kwerte = []
    for (xx,yy) in zip(x,y):
        kwerte.append(kwerte_position(I,T, xx, yy, korrelationsgroesse, maxdelta))
        count_kwerte+=1
        callback(count_kwerte * 0.3 / len(x))
        
    kwerte = np.array(kwerte, dtype=np.float)
    #for scale in np.arange(0.99, 1.01, 0.005):
    print "correlation values calculated"

    def alpha_durchgehen(alpha_values, scale_values,  translation_stepsize, callback_base, callback_multiplier, dx = 0.0, dy = 0.0):
        n_count = len(scale_values) * len(alpha_values)
        count = 0

        for scale in scale_values:
            for alpha in alpha_values:
                alpha = float(alpha)

                from bildregistrierung_ng_ext import find_best_for_angle

                find_best_for_angle(xr, yr, dx, dy, kwerte, maxdelta, alpha, scale, translation_stepsize, result)
                #print "for angle ",alpha, "scale = ", scale ,"\n                                       result =",result


                count += 1
                callback(callback_base + callback_multiplier/n_count * count )

    # korrelation, dx, dy, alpha, and scale, starting values
    result=np.array([-10000,0,0,0,1],dtype=np.float)

    scale_values = [1.0]
    alpha_values = np.arange(-alpha_max, alpha_max, alpha_step_1px)
    alpha_durchgehen(alpha_values, scale_values, 2.0, 0.3, 0.2)
    print "finished rough pass through transformations"

    print "result = ", result
    best_alpha = result[3]
    best_dx, best_dy = result[1:3]
    alpha_values = np.arange(best_alpha-alpha_step_1px*3, best_alpha+alpha_step_1px*3, alpha_step_1px/5)
    alpha_durchgehen(alpha_values, scale_values, 0.5, 0.5, 0.2, dx=best_dx, dy=best_dy)
    print "finished fine pass through transformations"


    T_T = np.ndarray(T.shape, dtype=np.float)
    best_kwert = -100
    best_alpha = result[3]

    alpha_values = np.arange(best_alpha-alpha_step_1px, best_alpha+alpha_step_1px, alpha_step_1px/5)

    best_dx = result[1]
    best_dy = result[2]

    x_values = np.arange(best_dx - 2, best_dx+2, .5)
    y_values = np.arange(best_dy-2, best_dy+2, .5)

    count = 0
    n_count = len(x_values) * len(alpha_values)
    for scale in scale_values:
        for alpha in alpha_values:

            for dx in x_values:
                count += 1
                callback(0.7 + count * 0.3 / n_count)
                for dy in y_values:
                    trafo = np.array([dx, dy, alpha, scale], dtype=np.float)
                    transform(T, trafo, T_T)
                    kwert = correlation(T_T[border:-border,border:-border], I[border:-border, border:-border])
                    if kwert > best_kwert:
                        best_kwert = kwert
                        result[1:] = trafo

    print "finished exact pass through transformations"
    print "result = ", result

    return result[1:]

if __name__ == "__main__":
    import glob
    import sys
    filenames = sys.argv[1:3]
    print filenames


    I,T = [np.load(f) for f in filenames[:2]]

    # trafo: dx, dy, alpha, scale,     alpha in radians, scale relative, 1.0 no scaling
    trafo = transformation_berechnen(I,T, maxdelta=34, korrelationsgroesse=128, N=5)

    T_T = np.ndarray(T.shape, dtype=np.float)
    
    transform(T, trafo, T_T)


    np.save("bildregistrierung-ng-out0-no-trafo.npy", T)
    np.save("bildregistrierung-ng-out1.npy", T_T)
    np.save("bildregistrierung-ng-out2.npy", I)
