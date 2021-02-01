#!/usr/bin/python3

# Copyright (C) 2020-2021 Marcin Kelar

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

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