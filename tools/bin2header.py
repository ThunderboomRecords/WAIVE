# adapted from https://github.com/user-none/Bin-Headers (MIT License)

import os
import sys
import argparse


def bin2header(data, var_name='var'):
    out = []
    out.append('/*\n  generated with bin2header.py \n*/')
    out.append('const unsigned int {var_name}_len = {data_len};'.format(var_name=var_name, data_len=len(data)))
    out.append('const unsigned char {var_name}[] = {{'.format(var_name=var_name))
    l = [ data[i:i+12] for i in range(0, len(data), 12) ]
    for i, x in enumerate(l):
        line = ', '.join([f'0x{c:02x}' for c in x])
        if i < len(l) - 1:
            line = line + ','
        out.append("    " + line)
    out.append('};')
    return '\n'.join(out)


def fn2VarName(fn: str) -> str:
    # basename without extension
    bn = os.path.splitext(os.path.basename(fn))[0]
    
    # remove leading or trailing spaces
    bn = bn.strip()

    # replace all non alphanumeric with underscore
    bn = re.sub(r'[^a-zA-Z0-9]', '_', bn)

    # prepends underscore if first character is not digit
    if bn[0].isnumeric():
        bn = "_" + bn

    return bn


def main():
    parser = argparse.ArgumentParser(description='Generate binary header output')
    parser.add_argument('-i', '--input', required=True , help='Input file')
    parser.add_argument('-o', '--out', required=True , help='Output file')
    parser.add_argument('-v', '--var', help='Variable name to use in file')

    args = parser.parse_args()
    if not args:
        return 1

    with open(args.input, 'rb') as f:
        data = f.read()

    if not args.var:
        args.var = fn2VarName(args.input)

    out = bin2header(data, args.var)
    with open(args.out, 'w') as f:
        f.write(out)

    return 0

if __name__ == '__main__':
    sys.exit(main())
