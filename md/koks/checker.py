import sys
import hashlib

expected = sys.argv[1] if len(sys.argv) > 1 else None
data = sys.stdin.read()

# Split by double newlines into blocks
blocks = data.split("\n\n")

# Remove leading/trailing whitespace from each block, as well as empty lines
cleaned_blocks = []
for block in blocks:
    lines = block.strip().split("\n")
    lines = [line.strip() for line in lines if line.strip()]
    cleaned_blocks.append("\n".join(lines))

# Sort lines within each block
sorted_blocks = []
for block in cleaned_blocks:
    lines = block.split("\n")
    lines.sort()
    sorted_blocks.append("\n".join(lines))

# Sort blocks themselves
sorted_blocks.sort()

# Rejoin with double newlines
normalized = "\n\n".join(sorted_blocks)

# Compute sha256
digest = hashlib.sha256(normalized.encode("utf-8")).hexdigest()

if expected is None:
    print(digest)
else:
    print("correct" if digest == expected else "incorrect")