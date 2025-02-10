# Basic training with a single file
./aimarket --train <file> --type TEXT

# Batch training with a directory
./aimarket --batch-train <directory> --type TEXT

# Web crawling and training
./aimarket --crawl "https://example.com/article"
```

### Processing Commands

```bash
# Process a file
./aimarket --process <file> --type TEXT

# Check model status
./aimarket --status

# View model reasoning
./aimarket --reasoning
```

## Advanced Features

### 1. Batch Training with Advanced Options
```bash
# Train with all files in a directory
./aimarket --batch-train ./training_data --type TEXT

# Train with progress display
./aimarket --batch-train ./training_data --type TEXT --show-progress

# Export training metrics
./aimarket --batch-train ./training_data --type TEXT --export-metrics metrics.txt
```

### 2. Training Configuration
```bash
# Set custom learning rate
./aimarket --train data.txt --type TEXT --learning-rate 0.001

# Set batch size
./aimarket --train data.txt --type TEXT --batch-size 64

# Set number of iterations
./aimarket --train data.txt --type TEXT --iterations 500

# Combine multiple parameters
./aimarket --train data.txt --type TEXT \
    --learning-rate 0.001 \
    --batch-size 64 \
    --iterations 500 \
    --show-progress
```

### 3. Model Monitoring
```bash
# Show training progress
./aimarket --train data.txt --type TEXT --show-progress

# Export training metrics
./aimarket --train data.txt --type TEXT --export-metrics metrics.txt

# Monitor model status
./aimarket --status
```

## Basic Operations

### Building the Application
```bash
# Clean previous builds
make clean

# Build the application
make

# Verify installation
./aimarket --version
```

### Multi-Modal Training

1. Text Training
```bash
# Create a text file for training
echo "Sample training data" > training.txt

# Train the model
./aimarket --train training.txt --type TEXT

# Verify training results
./aimarket --status
./aimarket --reasoning
```

2. Image Training
```bash
# Train with image file
./aimarket --train sample.jpg --type IMAGE

# Process an image
./aimarket --process image.jpg --type IMAGE
```

3. Web Content Training
```bash
# Train using content from a webpage
./aimarket --crawl "https://example.com/article"

# Check training results
./aimarket --status
./aimarket --reasoning
```

### Processing Content
1. Text Processing
```bash
# Create input file
echo "Text to process" > input.txt

# Process the text
./aimarket --process input.txt --type TEXT
```

2. Image Processing
```bash
# Process an image
./aimarket --process image.jpg --type IMAGE
```

### Monitoring and Debugging
1. Check Agent Status
```bash
./aimarket --status
```

2. View Agent Reasoning
```bash
./aimarket --reasoning
```

3. Run Tests
```bash
./aimarket --test
```


## Example Workflows

### Advanced Text Training Workflow
```bash
# Create training directory
mkdir training_data
echo "Training text 1" > training_data/text1.txt
echo "Training text 2" > training_data/text2.txt

# Batch train with progress monitoring
./aimarket --batch-train training_data --type TEXT \
    --learning-rate 0.001 \
    --batch-size 64 \
    --iterations 500 \
    --show-progress \
    --export-metrics training_results.txt

# Check training results
cat training_results.txt
./aimarket --status
./aimarket --reasoning
```

### Advanced Image Training Workflow
```bash
# Prepare image training directory
mkdir image_data
cp *.jpg image_data/

# Batch train images
./aimarket --batch-train image_data --type IMAGE \
    --learning-rate 0.0005 \
    --batch-size 32 \
    --iterations 1000 \
    --show-progress

# Process new image
./aimarket --process test.jpg --type IMAGE