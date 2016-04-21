"""
    Author: Tyler Barrus
    email:  barrust@gmail.com

    Version: 0.1.2

    Purpose: Simple script to help build passwords

    License: MIT 2015

    URL: https://github.com/barrust/password_generators
"""

if __name__ == '__main__':
    doth = 'hashmap.h'
    dotc = 'hashmap.c'
    unified = './dist/hashmap.h'
    # pull everything in
    with open(doth, 'r') as f:
        hlines = f.readlines()
    with open(dotc, 'r') as f:
        clines = f.readlines()

    # pull things apart
    hend = hlines[-2:]
    hlines = hlines[:-2]
    # figure out where the actual code begins / no include or comments
    i = 0
    for line in clines:
        if not line.startswith('#define'):
            i = i + 1
        else:
            break
    clines = clines[i:]

    # put everything together
    with open(unified, 'w') as f:
        # define functions (old .h)
        for line in hlines:
            f.write(line)
        # define code (old .c)
        for line in clines:
            f.write(line)
        # complete the #define
        for line in hend:
            f.write(line)
