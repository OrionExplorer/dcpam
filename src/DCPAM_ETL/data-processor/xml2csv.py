#!/usr/bin/python3
import sys
import xml.etree.ElementTree as ET
import pandas as pd

if __name__ == '__main__':
    if len( sys.argv ) == 4:
        inputFileName = sys.argv[ 1 ]
        outputFileName = sys.argv[ 2 ]
        delimiter = sys.argv[ 3 ]

        tree = ET.parse( inputFileName )
        root = tree.getroot()

        get_range = lambda col: range(len(col))
        l = [{r[i].tag:r[i].text for i in get_range(r)} for r in root]

        df = pd.DataFrame.from_dict(l)
        df.to_csv( outputFileName, sep=delimiter, encoding='utf-8', index=False)
        sys.stdout.buffer.write( "1".encode('utf8') )
    else:
        sys.stdout.buffer.write( "0".encode('utf8') )
    sys.stdout.flush()