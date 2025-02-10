import markdownpdf from 'markdown-pdf';
import { fileURLToPath } from 'url';
import path from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Convert API documentation
markdownpdf()
  .from(path.join(__dirname, 'documentation.md'))
  .to(path.join(__dirname, 'AI_Model_Marketplace_Documentation.pdf'), function () {
    console.log('API Documentation converted to PDF successfully!');
  });

// Convert user guide
markdownpdf()
  .from(path.join(__dirname, 'user_guide.md'))
  .to(path.join(__dirname, 'AI_Model_Marketplace_User_Guide.pdf'), function () {
    console.log('User Guide converted to PDF successfully!');
  });

// Convert code guide
markdownpdf()
  .from(path.join(__dirname, 'guides/code_guide.md'))
  .to(path.join(__dirname, 'AI_Model_Marketplace_Code_Guide.pdf'), function () {
    console.log('Code Guide converted to PDF successfully!');
  });

// Convert text processing guide
markdownpdf()
  .from(path.join(__dirname, 'TEXT_PROCESSING.md'))
  .to(path.join(__dirname, 'AI_Model_Text_Processing_Guide.pdf'), function () {
    console.log('Text Processing Guide converted to PDF successfully!');
  });

// Convert changes documentation
markdownpdf()
  .from(path.join(__dirname, 'changes.md'))
  .to(path.join(__dirname, 'AI_Model_Changes_Documentation.pdf'), function () {
    console.log('Changes Documentation converted to PDF successfully!');
  });