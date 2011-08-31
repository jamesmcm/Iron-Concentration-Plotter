import scipy.interpolate
#note that times assumed to be given in microseconds

#define constants, not all of these used but could be used to calculate p if wished
#q=1.6E-19
#kTq=0.025851
#Nv=3.1E19
#Nc=2.86E19
#Eg=1.1241
#vth=1.1E7
defaultconstants=[1.1E7,1.3E-14,1.28E13,7E-17,5E-15,1.22571E15,3E-15]

#constants which could be modified
#sigmani=1.3E-14
#p1i=1.28E13
#sigmapi=7E-17
#sigmanb=5E-15
#sigmapb=3E-15
#n1b=1.22571E15

def calcPrefactor(constants, NA, deltaN):
    xi=((constants[0]*(NA+deltaN))/(((1/constants[1])*(NA+constants[2]+deltaN))+(deltaN/constants[3])))
    xb=((constants[0]*(NA+deltaN))/(((1/constants[4])*(NA+deltaN))+((deltaN+constants[5])/constants[6])))
    C=1/(xi - xb)
    #it is a million too small due to seconds being taken in microseconds
    C=C*1E6
    return C

def calcFeConc(constants, NA, deltaN, tB, ti):
    C=calcPrefactor(constants, NA, deltaN)
    #print C
    FeConc=C*((1/ti)-(1/tB))
    return FeConc

def interpolation(beforelist, afterlist):
    #perhaps use truncated list
    #interpolate one with respect to the other and vice versa
    # combine the itnerpolated points with their respected line using a dictionary so it can be sorted for x values
    # return a sorted dictionary with one x value, two y values
    #bur cannot sort dictionary so instead make a list of sorted keys (x values) and this is used as an interface to the dictionary
    # x coords (deltan) are 2, y (tau) coords are 1
    #can use left= and right= in interp arguments to cull limits that produce nonsense
    #lists are backwardsWTF

 #can move this higher up later
    binterp=scipy.interpolate.interp1d(beforelist[2], beforelist[1])
    ainterp=scipy.interpolate.interp1d(afterlist[2], afterlist[1])
    aobdict={}
    boadict={}
    origlength=len(beforelist[2])
    for value in afterlist[2]:
        if value>=beforelist[2][0] and value<=beforelist[2][origlength-1]:
            aobdict[value]=binterp(value)
    for value in beforelist[2]:
        if value>=afterlist[2][0] and value<=afterlist[2][origlength-1]:
            boadict[value]=ainterp(value)

    #print afterlist[2]
    #print beforelist[2]
    #print beforelist[1]
    #print afteronbeforelist
    bdict=dict(zip(beforelist[2], beforelist[1]))
    totalbeforeplotdict=dict(aobdict.items()+bdict.items())
    #for key in sorted(totalbeforeplotdict.iterkeys()):
    #    print "%e: %e" % (key, totalbeforeplotdict[key])
    adict=dict(zip(afterlist[2], afterlist[1]))
    totalafterplotdict=dict(boadict.items()+adict.items())
    dictlist=[totalbeforeplotdict, totalafterplotdict]
    return dictlist

def COPcalc(constants, dope):
    #calculate theoretical COP value use eqn from JAPL 97, 103708
    #try ignoring T dependence first to see how bad it is
    #doping concentration must be greater than 10^14
    cop=(((1/constants[4])-(1/constants[1]))/(1/constants[3]))*dope
    return cop

if __name__ == "__main__":
    import sys
    print calcFeConc(defaultconstants, float(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4]))
