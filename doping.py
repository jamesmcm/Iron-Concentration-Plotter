from math import exp, log, pow
from scipy.optimize import newton

def calcDoping(rho):
    r=log(rho)
    pMu= 482.8 / (1 + 0.1322 / pow(rho, 0.811))
    if rho <= 0.1:
        pMu = pMu + 52.4 * exp((-1*rho)/0.00409)
    return exp(43.28)*(1.0/(pMu*rho))
    #return exp(43.28 - r - log(pMu))



def calcRes(NA):
    #defined inside so can grab NA value

    def f(x):
        try:
            return pow(x,-1) + (0.1322*pow(x,-1.811)) - ((482.8*NA)/exp(43.28))
        except:
            return  -1*((482.8*NA)/exp(43.28))
    #def fprime(x):
    #    return pow(x,-2) - (0.2394142*pow(x, -2.811))
    x0=1
    return newton(f,x0)

    
if __name__ == "__main__":
    import sys
    print calcDoping(float(sys.argv[1]))
    print calcRes(calcDoping(float(sys.argv[1])))
