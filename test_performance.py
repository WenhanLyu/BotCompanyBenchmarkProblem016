#!/usr/bin/env python3
import random
import sys

def generate_test(num_ops, seed=42):
    random.seed(seed)
    keys = [f"key{i:06d}" for i in range(num_ops // 3)]
    
    operations = []
    operations.append(str(num_ops))
    
    for _ in range(num_ops):
        op = random.choice(['insert', 'find', 'delete'])
        key = random.choice(keys)
        
        if op == 'insert':
            value = random.randint(1, 1000000)
            operations.append(f"insert {key} {value}")
        elif op == 'find':
            operations.append(f"find {key}")
        else:
            value = random.randint(1, 1000000)
            operations.append(f"delete {key} {value}")
    
    return '\n'.join(operations)

if __name__ == '__main__':
    num_ops = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    print(generate_test(num_ops))
