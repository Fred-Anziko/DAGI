
# Enhanced Web Crawler Documentation

## New Features

### 1. Goal-Oriented Crawling
- Takes a goal/objective as input to guide the crawling process
- Evaluates content relevance based on goal keywords
- Returns relevance scores for found content

### 2. Interactive Progress Updates
- Real-time progress reporting
- Pages crawled counter
- Error notifications
- Relevance score feedback

### 3. Form Handling
- Automatic form detection and filling
- Handles common input types (text, email, password)
- Form submission with wait states
- Error handling for form interactions

### 4. Content Management
- Saves relevant content to separate files
- Organizes content by relevance scores
- Maintains crawl depth control
- Domain-restricted crawling

## Usage

```bash
# Basic crawling
./aimarket --crawl "https://example.com"

# Goal-oriented crawling
./aimarket --crawl "https://example.com" "find information about machine learning"
```

## Output Format

### Progress Updates
```json
{
    "type": "progress",
    "message": "Crawling https://example.com",
    "pages_crawled": 1,
    "total_pages": 100
}
```

### Results
```json
{
    "type": "complete",
    "success": true,
    "results": [
        {
            "url": "https://example.com/page1",
            "relevance": 0.75,
            "file": "temp/crawled_data_1.txt"
        }
    ]
}
```

## Configuration

- Maximum depth: 3 levels
- Maximum pages: 100
- Relevance threshold: 0.5
- Domain restriction: Same as base URL
