// app.js - RESTful API server using http: 
// @MarquetteU   F Frigo  09-Feb-2022
//
// To start server:  $ node app.js
// For client use web browser:  http://localhost:3000
//
//
const express = require('express')
const app = express()
const port = 3000

app.get('/', (req, res) => {
  res.send('Hello World!')
})

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
})
