./aimarket --crawl "https://example.com/article"
```

This will:
1. Crawl the specified webpage
2. Extract meaningful text content
3. Use the content to train the model
4. Provide feedback on training accuracy

### Web Crawling Features
- Automatic text extraction from web pages
- Handles various website formats
- Filters out boilerplate content
- Focuses on main article content

### Example Usage:

1. Train with web content:
```bash
./aimarket --crawl "https://example.com/article"
```

2. Verify training results:
```bash
./aimarket --status
./aimarket --reasoning
```

## Existing Text Processing Features

The application supports two main operations for text:
1. Training the model with text data
2. Processing text using the trained model

### Training the Model

To train the model with text data:

```bash
./aimarket --train <text_file> --type TEXT
```

Example:
```bash
echo "Training data goes here" > training.txt
./aimarket --train training.txt --type TEXT
```

The training process will:
- Load the text data
- Update the model weights
- Provide feedback on the training accuracy
- Update the model version

### Processing Text

To process text using the trained model:

```bash
./aimarket --process <text_file> --type TEXT
```

Example:
```bash
echo "Text to process" > input.txt
./aimarket --process input.txt --type TEXT
```

## Command Reference

### Available Commands

1. `--train`: Train the model with input data
   ```bash
   ./aimarket --train <file> --type TEXT
   ```

2. `--process`: Process input using the trained model
   ```bash
   ./aimarket --process <file> --type TEXT
   ```

3. `--status`: Check the current state of the agent
   ```bash
   ./aimarket --status
   ```

4. `--reasoning`: Get the agent's reasoning for its last action
   ```bash
   ./aimarket --reasoning
   ```

5. `--help`: Display help information
   ```bash
   ./aimarket --help
   ```

6. `--version`: Display version information
   ```bash
   ./aimarket --version
   ```

7. `--crawl`: Crawl a webpage and use its content for training
   ```bash
   ./aimarket --crawl <url>
   ```


### Command Line Arguments

- `--type TEXT`: Specifies text processing mode
- `<file>`: Path to the input file
- `<url>`: URL of the webpage to crawl
- No additional flags needed for status, reasoning, help, or version commands

## Examples

### Complete Training Workflow

1. Create training data:
```bash
echo "This is sample training data for the AI model. It will help the model learn text patterns." > training.txt
```

2. Train the model:
```bash
./aimarket --train training.txt --type TEXT
```

3. Check training results:
```bash
./aimarket --reasoning
```

### Complete Processing Workflow

1. Create input text:
```bash
echo "This is a test sentence to be processed by our trained model." > test.txt
```

2. Process the text:
```bash
./aimarket --process test.txt --type TEXT
```

3. View processing results:
```bash
./aimarket --reasoning
```

### Complete Web Crawling Workflow

1. Train with web content:
```bash
./aimarket --crawl "https://example.com/article"
```

2. Verify training results:
```bash
./aimarket --status
./aimarket --reasoning
```

## Output Understanding

### Training Output

When training, you'll see output like:
```
Training complete. Accuracy: 0.0560503
Feedback: Selected action: 1 for media type 1 because: Model accuracy can be improved
```

This shows:
- Current model accuracy
- Agent's reasoning for the training action
- Model version information

### Processing Output

When processing text, you'll see output like:
```
Processed result: Selected action: 1 for media type 1 because: ...
```

This includes:
- Processing result
- Agent's reasoning for the processing action

## Troubleshooting

### Common Issues

1. File Permission Issues
```bash
# Fix with
chmod +x aimarket
```

2. Build Errors
```bash
# Clean and rebuild
make clean
make