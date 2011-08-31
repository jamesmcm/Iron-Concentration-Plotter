from math import exp, log, pow

def calcDoping(rho):
    r=log(rho)
    pMu= 482.8 / (1 + 0.1322 / pow(rho, 0.811))
    if rho <= 0.1:
        pMu = pMu + 52.4 * exp((-1*rho)/0.00409)
    return exp(43.28 - r - log(pMu))


if __name__ == "__main__":
    import sys
    print calcDoping(float(sys.argv[1]))
