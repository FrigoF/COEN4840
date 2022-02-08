// fred.js - RESTful API server using TLS
// @MarquetteU   F Frigo  09-Feb-2022
//
// To start server:  $ node fred.js
// For client use web browser:  https://bloomcounty.eng.mu.edu:3000/fred
//
const fs = require('fs');
const https = require('https');
const express = require('express');
const port = 3000
const app = express();

app.get('/fred', (req, res) => {
  return res.send('Hello, fred!');
});

https
  .createServer(
    {
      // ...
      cert: fs.readFileSync('/etc/apache2/ssl/bloomcounty_eng_mu_edu_cert.pem'),
      key: fs.readFileSync('/etc/apache2/ssl/privateKey.pem'),
      // ...
    },
    app
  )
  .listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})
