#!/usr/bin/python3
import sys
import pandas as pd

if __name__ == '__main__':
    if len( sys.argv ) == 3:
        inputFileName = sys.argv[ 1 ]
        outputFileName = sys.argv[ 2 ]
        xlsxFile = pd.read_excel( inputFileName, dtype=str, index_col=None )
        xlsxFile.to_csv( outputFileName, sep=';', encoding='utf-8', index=False )
        sys.stdout.buffer.write( "1".encode('utf8') )
    else:
        sys.stdout.buffer.write( "0".encode('utf8') )
    sys.stdout.flush()