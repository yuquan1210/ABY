
op = ['AND', 'MUL']
protocol = ['Y', 'B']

# 1. for each gate, specific gate type (name), inleft id, inright id, out id (this gate's id) (similar to how we write high-level program in C)

# 2. write a program that constructs high level circuit (find out which layer this gate is placed, based on above)
# 3. write a program that for each target gate, find its neighbors (parallel or sequential)
    # for parallel neighbors
    # orig comm of this target gate as base, add in neighbor gates' parallel communications (consider most parallel and least parallel?)
        # parallel neighbors using different protocol can be placed in parallel
        # parallel neighbors (no data dependency) do not spread out (tried y), they accumulate on same layer (done w/ computation as soon as possible, least depth)
    # for sequential neighbors
    # consider most parallel and least parallel
    # 
hl_circ = [
    ['MUL', 1, 2, 3], #how to show data dependency
    ['MUL'],
    ['AND'], #layer 1
] 

# assume data dependency is given, 
parallel_neighbors = []
sequential_top_neighbors = [
    ['MUL'],
    ['MUL']
]

# calculate cost for AND-B, knowing neighbors
# calculate most-overlapped rounds when sequential gates all use same protocol
for each_layer in sequential_top_neighbors:
    for each_op in each_layer:
        # get max depth of circuit (of same protocol) to determine whether AND use min_spreadout or max_spreadout
        # read AND most spreadout, use it as base, gradually add in overlapping based on neighbor layer


# calculate least-overlapped rounds when immediate sequential gate is conversion

# Note: must consider parallel (parallel gates are the key contributors, sequential ones are not key)
# sequential ones are not significant contributor because once there is conversion, minimum influence from top/bottom (need confirm with other conversions?)
