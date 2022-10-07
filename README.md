# OrdMatchingEngine

Matching Engine with only 1 order book, more order books can be added upon requests. This application provides an interactive menu to send and view orders for up to 3 clients.
Prices can support up to 2 decimal places. To send market order just set the price to be 0.

## Build

Need CMake and g++ 14 compiler to build.
	cmake .
	make

I used Visual Studio to build and run it.

## Menu

1. Select client - There are predefined 3 clients to send orders to the matcing engine (ME). First select the active client to send orders.

2. Dump order book - this option to print the entire order book

3. Create order - create a New order for the selected client, choose the side,price,qty for the order to subnit.

4. Cancel order - create a Cancel order for the selected client, chhose the order to cancel using the orderId.

5. Quit - Quit the application
