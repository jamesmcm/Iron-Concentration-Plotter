#define constants, not all of these used but could be used to calculate p if wished
q=1.6E-19
kTq=0.025851
Nv=3.1E19
Nc=2.86E19
Eg=1.1241
vth=1.1E7

#constants which could be modified
sigmani=1.3E-14
p1i=1.28E13
sigmapi=7E-17
sigmanb=5E-15
sigmapb=3E-15
n1b=1.22571E15



def calcPrefactor(NA, deltaN):
    xi=((vth*(NA+deltaN))/(((1/sigmani)*(NA+p1i+deltaN))+(deltaN/sigmapi)))
    xb=((vth*(NA+deltaN))/(((1/sigmanb)*(NA+deltaN))+((deltaN+n1b)/sigmapb)))
    C=1/(xi - xb)
#hack because it is always a million too small compared to the spreadsheet? Fix this later ideally - perhaps related to microsecond time issue
    C=C*1E6
    return C

if __name__ == "__main__":
    import sys
    print calcPrefactor(float(sys.argv[1]), float(sys.argv[2]))


