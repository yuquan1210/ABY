import csv
from io import StringIO
from collections import defaultdict
import json

# Read the CSV file
def read_circuit_csv(file_path):
    with open(file_path, mode='r', newline='') as file:
        csv_reader = csv.reader(file)

        # Read the header row and process it to handle duplicate headers
        original_header = next(csv_reader)
        headers = []
        headers_count = defaultdict(int)
        for h in original_header:
            headers_count[h] += 1
            if headers_count[h] > 1:
                new_name = f"{h}_{headers_count[h]}"
            else:
                new_name = h
            headers.append(new_name)

        # Initialize a data structure to hold the data layer by layer
        layers = []

        # Process each row in the CSV file
        for row in csv_reader:
            # Create a dictionary for each layer using the processed header
            layer_data = {headers[i]: float(row[i]) for i in range(1, len(row))}
            layers.append(layer_data)

    # Display the data structure
    # for layer in layers:
    #     print(layer)
    return headers, layers

def read_cost(file_path):
    # Initialize a list to hold the timing values
    timing_list = []

    # Open the file and read the data
    with open(file_path, 'r') as file:
        for line in file:
            # Strip whitespace and split each line on the comma
            parts = line.strip().split(',')
            # Convert the index and timing value to the appropriate types
            # index = int(parts[0])
            # timing_value = float(parts[2])

            # # Ensure the list is large enough to hold the current index
            # if len(timing_list) <= index:
            #     # Extend the list with None values if necessary
            #     timing_list.extend([None] * (index + 1 - len(timing_list)))
            
            # # Assign the timing value to the specified index
            # timing_list[index] = timing_value
            timing_list.append(float(parts[0]))

    return timing_list


def calculate_cost_table(headers, schedule, costs):
    cost_table = {}
    for gate_name in headers[1:]:
        cost_table[gate_name] = {}
        gate_cost_sum = 0
        for layer_index in range(len(schedule)):
            gate_data = schedule[layer_index][gate_name]
            total_data = sum(schedule[layer_index].values())
            gate_ratio = gate_data / total_data
            
            gate_layer_cost = costs[layer_index]
            gate_cost_sum += gate_layer_cost * gate_ratio
        cost_table[gate_name] = gate_cost_sum
    return cost_table


# File path
headers, schedule = read_circuit_csv('/home/ethan/MPC/ABY/circ_graph/biometric.csv')
costs = read_cost('/home/ethan/MPC/ABY/bio_b_real_cost.txt')
cost_table = calculate_cost_table(headers, schedule, costs)
print(f"real cost table: {cost_table}")

with open("/home/ethan/MPC/ABY/bio_b_real_costtable.json", 'w') as file:
    json.dump(cost_table, file, indent=4)


