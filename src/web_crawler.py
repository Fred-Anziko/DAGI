
import trafilatura
import sys
import json
from pathlib import Path
from urllib.parse import urljoin, urlparse
import requests
from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import time

class WebCrawler:
    def __init__(self, max_depth=3, max_pages=100, goal=None):
        self.visited_urls = set()
        self.max_depth = max_depth
        self.max_pages = max_pages
        self.current_pages = 0
        self.goal = goal
        self.results = []
        
    def is_valid_url(self, url, base_url):
        parsed_base = urlparse(base_url)
        parsed_url = urlparse(url)
        return parsed_url.netloc == parsed_base.netloc

    def evaluate_content(self, text_content):
        if not self.goal or not text_content:
            return 0
        # Simple relevance score based on goal keywords
        keywords = self.goal.lower().split()
        text_lower = text_content.lower()
        score = sum(1 for keyword in keywords if keyword in text_lower)
        return score / len(keywords) if keywords else 0
        
    def handle_forms(self, driver, url):
        forms = driver.find_elements(By.TAG_NAME, "form")
        for form in forms:
            try:
                inputs = form.find_elements(By.TAG_NAME, "input")
                for input_field in inputs:
                    input_type = input_field.get_attribute("type")
                    if input_type == "text":
                        input_field.send_keys("test")
                    elif input_type == "email":
                        input_field.send_keys("test@example.com")
                    elif input_type == "password":
                        input_field.send_keys("password123")
                
                submit = form.find_element(By.CSS_SELECTOR, "input[type='submit'], button[type='submit']")
                if submit:
                    submit.click()
                    WebDriverWait(driver, 10).until(EC.staleness_of(form))
                    self.print_progress(f"Submitted form on {url}")
            except Exception as e:
                self.print_progress(f"Form handling error on {url}: {str(e)}")

    def print_progress(self, message):
        progress = {
            "type": "progress",
            "message": message,
            "pages_crawled": len(self.visited_urls),
            "total_pages": self.max_pages
        }
        print(json.dumps(progress), flush=True)

    def crawl(self, url, depth=0):
        if depth > self.max_depth or url in self.visited_urls or self.current_pages >= self.max_pages:
            return []
        
        self.visited_urls.add(url)
        self.current_pages += 1
        self.print_progress(f"Crawling {url}")
        
        try:
            options = webdriver.FirefoxOptions()
            options.add_argument('--headless')
            driver = webdriver.Firefox(options=options)
            driver.get(url)
            
            self.handle_forms(driver, url)
            page_source = driver.page_source
            driver.quit()
            
            text_content = trafilatura.extract(page_source)
            relevance_score = self.evaluate_content(text_content)
            
            if relevance_score > 0.5:  # Threshold for relevant content
                output_path = f"temp/crawled_data_{len(self.visited_urls)}.txt"
                Path("temp").mkdir(exist_ok=True)
                with open(output_path, 'w', encoding='utf-8') as f:
                    f.write(text_content if text_content else '')
                
                result = {
                    "url": url,
                    "relevance": relevance_score,
                    "file": output_path
                }
                self.results.append(result)
                self.print_progress(f"Found relevant content (score: {relevance_score:.2f})")
            
            soup = BeautifulSoup(page_source, 'html.parser')
            links = [urljoin(url, a.get('href')) for a in soup.find_all('a', href=True)]
            
            for link in links:
                if self.is_valid_url(link, url):
                    self.crawl(link, depth + 1)
            
            return self.results
            
        except Exception as e:
            self.print_progress(f"Error crawling {url}: {str(e)}")
            return self.results

def save_crawled_content(url: str, goal: str = None):
    try:
        crawler = WebCrawler(goal=goal)
        results = crawler.crawl(url)
        
        print(json.dumps({
            "type": "complete",
            "success": True,
            "message": "Crawling completed",
            "results": results
        }), flush=True)
        
    except Exception as e:
        print(json.dumps({
            "type": "error",
            "success": False,
            "error": str(e)
        }), flush=True)
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({
            "type": "error",
            "success": False,
            "error": "URL argument is required"
        }))
        sys.exit(1)
        
    url = sys.argv[1]
    goal = sys.argv[2] if len(sys.argv) > 2 else None
    save_crawled_content(url, goal)
