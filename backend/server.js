const express = require('express');
const cors = require('cors');
const fs = require('fs');
const csv = require('csv-parser');
const { parse } = require('json2csv');

const app = express();

app.use(cors());
app.use(express.json());

let orders = [];
fs.createReadStream('orders.csv')
  .pipe(csv())
  .on('data', (row) => {
    orders.push(row);
  });

app.post('/submit-order', (req, res) => {
  const { clientOrderID, instrument, side, price, quantity } = req.body;

  if (quantity % 10 !== 0 || quantity < 10 || quantity > 1000 || price <= 0) {
    return res.status(400).send('Invalid order');
  }

  const newOrder = {
    clientOrderID,
    instrument,
    side,
    price,
    quantity
  };
  orders.push(newOrder);

  const csvData = parse(orders);
  fs.writeFileSync('orders.csv', csvData);

  res.status(200).send('Order submitted');
});

app.get('/orders', (req, res) => {
  res.status(200).json(orders);
});

app.get('/execution-reports', (req, res) => {
  const executionReport = {
    clientOrderID: orders[0].clientOrderID,
    orderID: 'ORD123',
    status: 'Executed',
    transactionTime: new Date().toISOString()
  };

  res.status(200).json(executionReport);
});

app.listen(3000, () => {
  console.log('Server is running on port 3000');
});
