'''
    pull the c code into the header
'''
if __name__ == '__main__':
    doth = './src/hashmap.h'
    dotc = './src/hashmap.c'
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
