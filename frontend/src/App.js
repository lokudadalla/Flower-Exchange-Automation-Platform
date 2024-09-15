import React, { useState } from 'react';
import { TextField, Button, Select, MenuItem, Typography, Container } from '@mui/material';
import OrdersChart from './OrdersChart'; 
import './App.css';


function App() {
  const [order, setOrder] = useState({
    clientOrderID: '',
    instrument: 'Rose',
    side: 1,
    price: '',
    quantity: ''
  });

  const [message, setMessage] = useState('');
  const [orders, setOrders] = useState([]);

  const handleChange = (e) => {
    const { name, value } = e.target;
    setOrder((prevOrder) => ({
      ...prevOrder,
      [name]: value
    }));
  };

  const submitOrder = async () => {
    const response = await fetch('http://localhost:3000/submit-order', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(order)
    });
    const data = await response.text();
    setMessage(data);
  };

  const fetchOrders = async () => {
    const response = await fetch('http://localhost:3000/orders');
    const data = await response.json();
    setOrders(data);
  };

  return (
    <Container>
      <Typography variant="h4" gutterBottom>
        Flower Exchange System
      </Typography>

      <TextField
        label="Client Order ID"
        variant="outlined"
        name="clientOrderID"
        value={order.clientOrderID}
        onChange={handleChange}
        fullWidth
        margin="normal"
      />

      <Select
        name="instrument"
        value={order.instrument}
        onChange={handleChange}
        fullWidth
        margin="normal"
      >
        <MenuItem value="Rose">Rose</MenuItem>
        <MenuItem value="Lavender">Lavender</MenuItem>
        <MenuItem value="Lotus">Lotus</MenuItem>
        <MenuItem value="Tulip">Tulip</MenuItem>
        <MenuItem value="Orchid">Orchid</MenuItem>
      </Select>

      <Select
        name="side"
        value={order.side}
        onChange={handleChange}
        fullWidth
        margin="normal"
      >
        <MenuItem value={1}>Buy</MenuItem>
        <MenuItem value={2}>Sell</MenuItem>
      </Select>

      <TextField
        label="Price"
        variant="outlined"
        type="number"
        name="price"
        value={order.price}
        onChange={handleChange}
        fullWidth
        margin="normal"
      />

      <TextField
        label="Quantity"
        variant="outlined"
        type="number"
        name="quantity"
        value={order.quantity}
        onChange={handleChange}
        fullWidth
        margin="normal"
      />

      <Button variant="contained" color="primary" onClick={submitOrder} fullWidth>
        Submit Order
      </Button>

      <Typography variant="body1" color="secondary" gutterBottom>
        {message}
      </Typography>

      <Button variant="outlined" color="primary" onClick={fetchOrders} fullWidth>
        Fetch Orders
      </Button>

      <OrdersChart orders={orders} />
    </Container>
  );
}

export default App;
