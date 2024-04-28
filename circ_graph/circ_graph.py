import sys
import pandas as pd
import matplotlib.pyplot as plt
import networkx as nx

hl_gate_set = set()
hl_gate_num = 0

# Load data from CSV
def load_data(file_path):
    return pd.read_csv(file_path)

# Create the graph from the dataframe
def create_graph(df):
    global hl_gate_set
    global hl_gate_num

    G = nx.DiGraph()
    positions = {}
    layers = {}

    # add gates to G
    for _, gate in df.iterrows():
        G.add_node(gate['ll_gate_id'], hl_gate_id=gate['hl_gate_id'], hl_gate_name=gate['hl_gate_name'], sharingsize=gate['sharingsize'])
        if(gate['hl_gate_id'] not in hl_gate_set):
            hl_gate_set.add(gate['hl_gate_id'])
            hl_gate_num += 1


    # connect input output gates, and add to layer array
    for _, gate in df.iterrows():
        inputs = [gate['input1_id'], gate['input2_id']]  # Adjust if your CSV has different columns
        for input_id in inputs:
            G.add_edge(input_id, gate['ll_gate_id'])
        layer = gate['depth']

        if layer not in layers:
            layers[layer] = []
        layers[layer].append(gate['ll_gate_id'])
    
    # Position nodes and prepare for layer annotations
    y_pos = 0
    layer_positions = {}
    for layer in sorted(layers):
        x_positions = range(len(layers[layer]))
        for idx, node_id in enumerate(layers[layer]):
            positions[node_id] = (x_positions[idx] + 5, -y_pos)  # Offset x by 1 to make room for labels
        layer_positions[layer] = (-0.5, -y_pos)  # Set position for layer labels slightly left of the first node
        y_pos += 1

    return G, positions, layer_positions

# Drawing the graph with layer labels
def draw_graph(G, positions, layer_positions):
    color_map = {}
    for node_id in G.nodes():
        # color_map[node] = G.nodes[]
        hl_gate_id = G.nodes[node_id]['hl_gate_id']
        color = plt.cm.viridis(hl_gate_id / hl_gate_num)
        color_map[node_id] = color
    plt.figure(figsize=(150, 100))  # Adjust size as necessary
    nx.draw(G, pos=positions, with_labels=True, labels=nx.get_node_attributes(G, 'hl_gate_name'), node_color=[color_map[node] for node in G.nodes()], node_size=100, font_size=5)
    
    # Draw layer numbers
    for layer, pos in layer_positions.items():
        plt.text(pos[0], pos[1], f"Layer {layer}", fontsize=6, verticalalignment='center', horizontalalignment='right')

    plt.show()

# Main function to run the process
def main(file_path):
    df = load_data(file_path)
    G, positions, layer_positions = create_graph(df)
    draw_graph(G, positions, layer_positions)

# Replace 'your_file.csv' with the path to your CSV file
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <input>")
        sys.exit(1)
    
    input_arg = sys.argv[1]
    main(input_arg)
