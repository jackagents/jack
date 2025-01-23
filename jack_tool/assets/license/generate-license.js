const nunjucks = require('nunjucks');
const fs = require('fs');

// Load the template file
const template = fs.readFileSync('./assets/license/template.md.njk', 'utf-8');

const clientName = (() => {
  return 'CBDI EDIT Client';
})();

// Render the template using process.env
const output = nunjucks.renderString(template, {
  clientName,
});

// Write the output to a file
fs.writeFileSync('./assets/license/license.md', output, 'utf-8');
