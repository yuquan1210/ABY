import json
from collections import defaultdict, deque

def read_circ(filename):
    gates = {}
    try:
        with open(filename, 'r') as file:
            for line in file:
                if line.strip():  # Check if line is not empty
                    # Split the line into components
                    parts = line.split(',')
                    gate_info = {}
                    for part in parts:
                        key, value = part.strip().split(':')
                        value = value.strip()
                        if value == "None":
                            value = None
                        gate_info[key.strip()] = value
                    
                    # Use gate_id as the key in the main dictionary
                    gate_id = gate_info['gate_id']
                    gates[gate_id] = gate_info
    except Exception as e:
        print(f"An error occurred: {e}")
    return gates

def read_schedule(filename):
    # Open the JSON file for reading
    with open(filename, 'r') as file:
        # Load its content and convert it into a Python dictionary
        data = json.load(file)

    # Now you can use the data as a normal dictionary
    return data

def build_circuit_bottomup(gate_dict):
    # Find the output gates as the starting point
    output_gates = []
    for _, gate_info in gate_dict.items():
        if gate_info['gate_type'] == 'OUT':
            output_gates.append(gate_info)

    # Initialize layers and a queue for BFS
    layers = []
    queue = []

    # Initialize layer information for each gate
    gate_layer = {gate_info['gate_id']: None for _, gate_info in gate_dict.items()}

    # Start with output gates at layer 0
    for gate in output_gates:
        queue.append((gate, 0))

    # Process the queue
    while queue:
        current_gate, current_layer = queue.pop(0)
        
        # Assign the layer if not already assigned or found a lower layer number
        if gate_layer[current_gate['gate_id']] is None or current_layer < gate_layer[current_gate['gate_id']]:
            gate_layer[current_gate['gate_id']] = current_layer

            # Extend the layers list if needed
            if len(layers) <= current_layer:
                layers.append([])

            # Add the current gate to the corresponding layer
            layers[current_layer].append(current_gate)

            # Queue the input gates of the current gate
            if current_gate['gate_type'] != 'IN':
                if current_gate['in_left'] is not None:
                    queue.append((gate_dict[current_gate['in_left']], current_layer + 1))
                if current_gate['in_right'] is not None:
                    queue.append((gate_dict[current_gate['in_right']], current_layer + 1))
                if 'in_cmp' in current_gate and current_gate['in_cmp'] is not None:
                    queue.append((gate_dict[current_gate['in_cmp']], current_layer + 1))

    # Print the layers from inputs to outputs
    reversed_layers = layers[::-1]
    for i in range(0, len(reversed_layers)):
        print(f"Layer {i}: {[gate['gate_id'] for gate in reversed_layers[i]]}")
    
    return reversed_layers


def build_circuit_topdown(gate_dict):
    # Create a mapping from gate_id to gates that depend on this gate_id
    dependency_graph = defaultdict(list)
    for _, gate in gate_dict.items():
        if gate['gate_type'] != 'IN':
            dependency_graph[gate['in_left']].append(gate['gate_id'])
            dependency_graph[gate['in_right']].append(gate['gate_id']) # map gate to its child
            if 'in_cmp' in gate:
                dependency_graph[gate['in_cmp']].append(gate['gate_id'])
    
    # Initialize the first layer with all input gates
    result_layers = []
    curr_layer = [gate['gate_id'] for _, gate in gate_dict.items() if gate['gate_type'] == 'IN']
    result_layers.append(curr_layer)
    
    # Using a set to track gates that have been processed
    processed_gates = set(curr_layer)
    
    gate_layer = {gate_id: 1 for gate_id in curr_layer}  # Starting layer is 1 for inputs
    
    # Process layers until no new gates can be added
    while curr_layer:
        next_layer = []
        for gate_id in curr_layer:
            # Add all gates that depend on the current gate to the next layer
            for child_gate_id in dependency_graph[gate_id]:
                child_gate = gate_dict[child_gate_id]
                if (child_gate_id not in processed_gates) and (child_gate['in_left'] == None or child_gate['in_left'] in processed_gates) and (child_gate['in_right'] == None or child_gate['in_right'] in processed_gates):
                    # Check if all inputs of the dependent gate are processed or None
                    if 'in_cmp' in child_gate:
                        if (child_gate['in_cmp'] != None) and (child_gate['in_cmp'] not in processed_gates): continue # check if cmp_parent processed, if not, skip this child gate
                        cmp_parent_layer = gate_layer.get(child_gate['in_cmp'], 0) 
                        if cmp_parent_layer >= len(result_layers) + 1:
                            continue # check if processed cmp parent is in the same layer (if processed, then definitely assigned has gate_layer)
                    left_parent_layer = gate_layer.get(child_gate['in_left'], 0) # if parent is None, give zero
                    right_parent_layer = gate_layer.get(child_gate['in_right'], 0)
                    if left_parent_layer < len(result_layers) + 1 and right_parent_layer < len(result_layers) + 1:
                        # Ensure the child gate goes to the next layer only
                        if child_gate_id not in next_layer:
                            next_layer.append(child_gate_id)
                            processed_gates.add(child_gate_id)
                            gate_layer[child_gate_id] = len(result_layers) + 1
        
        if next_layer:
            result_layers.append(next_layer)
        curr_layer = next_layer
    
    # Print the circuit layer by layer
    for i, layer in enumerate(result_layers):
        print(f"Layer {i+1}: {layer}")
    
    # Validation to ensure all gates are in the circuit
    if len(processed_gates) != len(gate_dict):
        print("Error: Not all gates have been included in the circuit.")
        print("Missing gates:", set(gate_dict.keys()) - processed_gates)
    else:
        print("All gates are included in the circuit properly.")

    #convert gate_id to gate
    result_layers_gate =[] 
    for layer in result_layers:
        new_layer_gate = []
        for gate_id in layer:
            gate = gate_dict[gate_id]
            new_layer_gate.append(gate)
        result_layers_gate.append(new_layer_gate)
    
    return result_layers_gate

def is_ancestor(gate_id, target_gate_id, gate_dict):
    """
    Check if a gate_id is an ancestor of target_gate_id.
    """
    visited = set()  # To keep track of visited gates to avoid cycles

    def dfs(current_gate_id):
        if current_gate_id == gate_id:
            return True
        if current_gate_id in visited or current_gate_id not in gate_dict:
            return False

        visited.add(current_gate_id)
        gate = gate_dict[current_gate_id]

        # Check both the left and right inputs recursively
        left_ancestor = dfs(gate.get('in_left')) if gate.get('in_left') else False
        right_ancestor = dfs(gate.get('in_right')) if gate.get('in_right') else False

        return left_ancestor or right_ancestor

    # Start DFS from the target_gate_id
    return dfs(target_gate_id)

def get_sequential_neighbors(layers, target_gate_id, gate_dict, seq_range=2):
    # Find the target gate's layer
    target_gate = gate_dict[target_gate_id]
    target_layer_index = None
    for i, layer in enumerate(layers):
        if any(gate['gate_id'] == target_gate_id for gate in layer):
            target_layer_index = i
            break

    if target_layer_index is None:
        print("Gate not found in the layers.")
        return

    # Collect neighbors within two layers distance
    up_neighbors = [] # farthest -> closest
    low_neighbors = [] # closest -> farthest
    up_range = range(max(0, target_layer_index - 2), target_layer_index) # target_layer=4, range=(2, 4), 2, 3
    low_range = range(target_layer_index+1, min(len(layers), target_layer_index + 3)) # range=(5, 7), 5, 6

    for i in up_range:
        curr_layer_neighbor = []
        for gate in layers[i]:
            # Check direct dependencies
            if is_ancestor(gate['gate_id'], target_gate['gate_id'], gate_dict):
                curr_layer_neighbor.append(gate['gate_id'])
        up_neighbors.append(curr_layer_neighbor)

    for i in low_range:
        curr_layer_neighbor = []
        for gate in layers[i]:
            # Check direct dependencies
            if is_ancestor(target_gate['gate_id'], gate['gate_id'], gate_dict):
                curr_layer_neighbor.append(gate['gate_id'])
        low_neighbors.append(curr_layer_neighbor)

    # print(f"up sequential neighbors of {target_gate['gate_id']}:\n {up_neighbors}")
    # print(f"low sequential neighbors:\n {low_neighbors}")
    return (up_neighbors, low_neighbors)

def get_parallel_neighbors(layers, target_gate_id, gate_dict):
    target_gate = gate_dict[target_gate_id]
    target_layer_index = None
    for i, layer in enumerate(layers):
        if any(gate['gate_id'] == target_gate_id for gate in layer):
            target_layer_index = i
            break
    
    parallel_neighbors = []
    for gate in layers[target_layer_index]:
        if not is_ancestor(target_gate['gate_id'], gate['gate_id'], gate_dict) and not is_ancestor(gate['gate_id'], target_gate['gate_id'], gate_dict):
            parallel_neighbors.append(gate['gate_id'])
    # print(f"parallel neighbors of {target_gate_id}")
    # print(parallel_neighbors)
    return parallel_neighbors

def add_parallel_schedule(target_schedule, add_schedule, pos, divisor):
    for layer_index in range(len(target_schedule)):
        if (layer_index+pos) < len(add_schedule):
            if pos >= 0: # add up_sequential/parallel schedule starting from layer [layer_index+pos]
                target_schedule[layer_index] += (add_schedule[layer_index+pos] / divisor)
            if pos < 0: 
                if layer_index+pos >= 0:
                    target_schedule[layer_index] += (add_schedule[layer_index+pos] / divisor)
    return target_schedule

def compute_gate_communication_schedule(layers, target_gate_id, target_gate_protocol, gate_dict, init_schd_info):

    # get target_gate schedule
    target_gate = gate_dict[target_gate_id]
    target_schd = init_schd_info[target_gate['gate_type']][target_gate_protocol]['most_spreadout'].copy() # assume max rounds in large circuit
    parallel_neighbors = get_parallel_neighbors(layers, target_gate_id, gate_dict)
    up_seq_neighbors, low_seq_neighbors = get_sequential_neighbors(layers, target_gate_id, gate_dict)

    # add parallel neighbors
    result_schd = target_schd
    for p in parallel_neighbors:
        p_gate = gate_dict[p]
        p_schd_info = init_schd_info[p_gate['gate_type']]
        for protocol in p_schd_info['protocols']:
            p_schd = p_schd_info[protocol]['most_spreadout'].copy() #list is mutable values
            result_schd = add_parallel_schedule(result_schd, p_schd, 0, p_schd_info['num_protocols'])
    
    up_range = len(up_seq_neighbors) 
    for i in range(len(up_seq_neighbors)):
        curr_seq_layer = up_seq_neighbors[i]
        for s in curr_seq_layer:
            s_gate = gate_dict[s]
            s_schd_info = init_schd_info[s_gate['gate_type']]
            if (target_gate_protocol not in s_schd_info['protocols']): continue
            s_schd = s_schd_info[target_gate_protocol]['most_spreadout'].copy()
            result_schd = add_parallel_schedule(result_schd, s_schd, up_range-i, 2)

    low_range = len(low_seq_neighbors) 
    for i in range(len(low_seq_neighbors)):
        curr_seq_layer = low_seq_neighbors[i]
        for s in curr_seq_layer:
            s_gate = gate_dict[s]
            s_schd_info = init_schd_info[s_gate['gate_type']]
            if (target_gate_protocol not in s_schd_info['protocols']): continue
            s_schd = s_schd_info[target_gate_protocol]['most_spreadout'].copy()
            result_schd = add_parallel_schedule(result_schd, s_schd, -(i+1), 2)
    
    return result_schd


def compute_gate_communication_time(target_gate_id, target_gate_protocol, gate_protocol_schd, gate_dict, init_schd_info, net_char):
    target_gate = gate_dict[target_gate_id]
    target_schd = init_schd_info[target_gate['gate_type']][target_gate_protocol]['most_spreadout'].copy()

    gate_time_sum = 0
    gate_layer_times = [0] * len(target_schd)
    layer_times = [0] * len(target_schd)
    for i in range(len(target_schd)):
        init_data = target_schd[i]
        total_data = gate_protocol_schd[i]
        layer_time = net_char['RTT'] + total_data/net_char['Bandwidth']
        gate_ratio = 1
        if total_data > 0:
            gate_ratio = init_data/total_data
        gate_time = layer_time * gate_ratio

        layer_times[i]=layer_time
        gate_layer_times[i] = gate_time
        gate_time_sum += gate_time

    print(f"{target_gate} {target_gate_protocol} time: {gate_time_sum}")
    return gate_time_sum


def compute_cost_table(layers, net_char, gate_dict, init_schd_info):
    # iterate each high-level gate (gate_id), compute corresponding schedule
    cost_table = {}
    for gate_id, gate in gate_dict.items():
        gate_name = gate['gate_type'] + gate_id[-4:]
        cost_table[gate_name] = {}
        for protocol in init_schd_info[gate['gate_type']]['protocols']:
            gate_protocol_schd = compute_gate_communication_schedule(layers, gate_id, protocol, gate_dict, init_schd_info)
            print(f"{gate_name}-{protocol} schd: \n{gate_protocol_schd}")
            # calculate time
            gate_protocol_time = compute_gate_communication_time(gate_id, protocol, gate_protocol_schd, gate_dict, init_schd_info, net_char)
            cost_table[gate_name][protocol] = gate_protocol_time

    return cost_table


# Example usage
gate_dict = read_circ("/home/ethan/MPC/ABY/bio_circ.txt")
layers = build_circuit_topdown(gate_dict)
init_schd_info = read_schedule("/home/ethan/MPC/ABY/schedule.json")
net_char = {'RTT': 12.35202727, 'Bandwidth': 1636.470636} # bits/ms
cost_table = compute_cost_table(layers, net_char, gate_dict, init_schd_info)
print(f"cost table: {cost_table}")

with open("/home/ethan/MPC/ABY/10_bio_b_estimated_costtable.json", 'w') as file:
    json.dump(cost_table, file, indent=4)
