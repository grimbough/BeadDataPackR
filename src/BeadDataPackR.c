#include "BeadDataPackR.h"

/* function to round the values from the .locs file into the truncated versions that 
are found in the .txt file */
SEXP roundLocsFileValues(SEXP inputVector) {
    
    int i, digits;
    double x, *inVec;
    SEXP outputVector;
    
    inVec = REAL(inputVector);
    PROTECT(outputVector = allocVector(REALSXP, length(inputVector)));
    
    for(i = 0; i < length(inputVector); i++) {
        x = inVec[i];
        /* the precison of the rounding is determined by the integer part of the value */
        if(x >= 10000)
            digits = 2;
        else if (x >= 1000)
            digits = 3;
        else if (x >= 100)
            digits = 4;
        else if (x >= 10)
            digits = 5;
        else
            digits = 6;
        /* perform the rounding to the required precision */
        REAL(outputVector)[i] = round(inVec[i]*(pow(10,digits)))/pow(10,digits);
    }
    UNPROTECT(1);
    return(outputVector);
}

SEXP composeIntensityFlags(SEXP neg, SEXP large) {

    SEXP flags;
    int i, j, k, nBytes, *negResized, *largeResized;
    int byte;
	
    nBytes = ( (length(neg) - 1) / 4 ) + 1;
	
    /* assign the vectors and set all entries to zero */
    negResized = (int *) R_alloc(sizeof(int), nBytes * 4);
    memset(negResized, 0, sizeof(int) * nBytes * 4);
    largeResized = (int *) R_alloc(sizeof(int), nBytes * 4);
    memset(largeResized, 0, sizeof(int) * nBytes * 4);
    PROTECT(flags = allocVector(INTSXP, nBytes));	
	
    for(i = 0; i < length(neg); i++) {
	negResized[i] = INTEGER(neg)[i];
	largeResized[i] = INTEGER(large)[i];
    }
	
    j = 0;
    for(i = 0; i < nBytes; i++ ) {
	byte = 0;
	for(k = 3; k >= 0; k--) {
            byte += ( negResized[j] * pow(2, k*2) );
            byte += largeResized[j] * pow(2, (k*2 + 1));
            j++;
	}
	INTEGER(flags)[i] = byte;
    }

    UNPROTECT(1);
    return(flags);
}

SEXP int2Bits(SEXP flags) {
    
    SEXP res;
    int i, j, f, idx;
    
    PROTECT(res = allocMatrix(INTSXP, length(flags) * 4, 2));
    
    idx = 0;
    for(i = 0; i < length(flags); i++) {
     
        f = INTEGER(flags)[i];

        for(j = 3; j >= 0; j--) {
            (f & 0x1) ? (INTEGER(res)[length(flags)*4 + (idx+j)] = 1) : (INTEGER(res)[length(flags)*4 + (idx+j)] = 0);
            f >>= 1; 
            (f & 0x1) ? ( INTEGER(res)[idx+j] = 1 ) : (INTEGER(res)[idx+j] = 0);
            f >>= 1; 
        }
        idx = idx + 4;
    }
    
    UNPROTECT(1);
    return(res);
}



SEXP decodeInd(SEXP indices) {
 
    SEXP idxMatrix;
    int i, *idx, *im;
    
    idx = INTEGER(indices);
    PROTECT(idxMatrix = allocMatrix(INTSXP, length(indices), 2));
    im = INTEGER(idxMatrix);
    
    for(i = 0; i < length(indices); i++) {
        im[i + length(indices)] = idx[i] % 16;
        im[i] = (idx[i] - (im[i + length(indices)]) ) / 16;
    }
    
    UNPROTECT(1);
    return(idxMatrix);
}

SEXP adjustValues(SEXP mat) {
 
    SEXP res;
    int i, d, nrow, *m = INTEGER(mat);
    nrow = INTEGER(getAttrib(mat, R_DimSymbol))[0];
    PROTECT(res = allocVector(INTSXP, nrow));

    for(i = 0; i < nrow; i++) {
        
        d = m[nrow + i] - m[(2*nrow) + i];
            
        if(d > 7)
            d -= 15;
        else if (d < -7)
            d += 15;
            
        INTEGER(res)[i] = m[i] + d;
    }
    
    UNPROTECT(1);
    return(res);
}


SEXP returnTrueIndex(SEXP predX, SEXP predY, SEXP nrow) {
 
    SEXP trueIndex;
    int i, n = INTEGER(nrow)[0];
    
    PROTECT(trueIndex = allocVector(INTSXP, length(predX)));
    
    for(i = 0; i < length(predX); i++) {
        INTEGER(trueIndex)[i] = ( INTEGER(predX)[i] * n ) + INTEGER(predY)[i];
    }
    UNPROTECT(1);
    return(trueIndex);
}

/*  Convert bit strings into ints 
    Expects a matrix with 1 column per coordinate 
    and as many rows as there are bits */
SEXP bitsToInts(SEXP bitMatrix) {
    
    SEXP resInts;
    int i, j, nBits, width, *res, *bm;
      
    nBits = INTEGER(getAttrib(bitMatrix, R_DimSymbol))[0];
    width = INTEGER(getAttrib(bitMatrix, R_DimSymbol))[1];
    /* length is the same as the number of cols in the bitMatrix */
    PROTECT(resInts = allocVector(INTSXP, width));
    /* set up pointers to the SEXP objects */
    res = INTEGER(resInts);
    bm = INTEGER(bitMatrix);
    
    /* loop over each element */
    for(i = 0; i < width; i++) {
        res[i] = 0;
        /* loop through the bits */
        for(j = 0; j < nBits; j++) {
            res[i] += ( pow(2, j) * bm[ (i * nBits) + j ] );
        }
    }
    
    UNPROTECT(1);
    return(resInts);
}
    

