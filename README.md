# Concert Connect

A comprehensive concert booking management system built in C that provides a complete solution for managing concert events, user registrations, and seat reservations. The application features separate portals for customers and administrators, offering an intuitive command-line interface for all booking operations.

## Overview

Concert Connect is designed to simulate a real-world ticket booking system where customers can discover upcoming concerts, book seats, and manage their reservations, while administrators have full control over event management and booking oversight. The system implements core concepts of file I/O, data structures, and modular programming in C.

### Key Highlights

- **Dual Portal System**: Separate interfaces for customers and administrators with role-based access control
- **Real-time Seat Management**: Track available and booked seats with live updates
- **Persistent Storage**: All data is saved to text files, maintaining state across sessions
- **Comprehensive Admin Tools**: Full event CRUD operations, analytics, and booking management
- **User-Friendly Interface**: Clean, menu-driven navigation with input validation

## Features

### User Management
- **Registration System**: Create new accounts with unique usernames and secure passwords
- **Role-Based Access**: Two user types - Customer and Admin - with distinct permissions
- **Authentication**: Login system with credential verification
- **Session Management**: Maintains user context throughout the application session

### Event Management (Admin)
- **Create Events**: Add new concerts with details including:
  - Event name and description
  - Date and time
  - Venue information
  - Total seat capacity
  - Ticket pricing
- **Edit Events**: Modify existing event details
- **Delete Events**: Remove events and handle associated bookings
- **Event Listing**: View all events with availability status

### Seat Booking System (Customer)
- **Browse Events**: View all available concerts with details
- **Real-time Availability**: Check seat availability before booking
- **Book Seats**: Reserve tickets for desired events
- **Multiple Bookings**: Support for booking multiple events per user
- **Booking Confirmation**: Receive booking ID and details upon successful reservation

### Booking Management
- **View Personal Bookings**: Customers can see their booking history
- **Cancel Bookings**: Option to cancel reservations with seat updates
- **Admin Overview**: Administrators can view all bookings across all events
- **Booking Search**: Search functionality for finding specific bookings
- **Analytics**: Booking statistics and insights for administrators

### Additional Features
- **Waiting Queue**: Queue system for fully booked events
- **Seat Map Visualization**: Visual representation of booked/available seats
- **Customer Database**: Admin access to registered user information
- **Price Management**: Dynamic ticket price adjustments
- **Data Persistence**: Automatic saving of all changes to file system

## Project Structure

The project follows a modular architecture with clear separation of concerns:

```
├── main.c          # Main application entry point and portal flows
├── users.c/h       # User management (registration, login, authentication)
├── events.c/h      # Event management (add, edit, delete events)
├── bookings.c/h    # Booking system (book, cancel, view bookings)
├── utils.c/h       # Utility functions (input handling, UI helpers)
├── Makefile        # Build configuration
└── README.md       # This file
```

### Module Descriptions

- **main.c**: Contains the main program loop, menu systems, and orchestrates the customer and admin portal flows
- **users.c/h**: Handles all user-related operations including registration, authentication, and user data management
- **events.c/h**: Manages concert events with functions for creating, editing, deleting, and querying event information
- **bookings.c/h**: Implements the core booking logic, seat allocation, cancellation, and booking queries
- **utils.c/h**: Provides utility functions for input validation, screen formatting, and common operations used across modules

## Requirements

- GCC compiler (or any C99 compatible compiler)
- Make

## Building

To compile the project:

```bash
make
```

This will create an executable named `concert_booking`.

## Running

To run the application:

```bash
./concert_booking
```

## Usage

### Getting Started

1. Launch the application using `./concert_booking`
2. You'll see the main menu with options to login or register
3. Choose whether to register as a Customer or Admin
4. After registration/login, you'll be directed to the appropriate portal

### Customer Workflow

The customer portal provides an intuitive interface for concert-goers:

1. **Book Seats**
   - View list of available concerts
   - Select an event by number
   - Choose number of seats to book
   - Receive booking confirmation with unique booking ID

2. **Cancel Bookings**
   - View your existing bookings
   - Select an event to cancel
   - Confirm cancellation
   - Seats are automatically released back to availability

3. **View My Bookings**
   - See all your current reservations
   - View event details, booking IDs, and seat counts
   - Track your upcoming concerts

### Admin Workflow

The admin portal offers comprehensive management capabilities:

1. **Event Management**
   - List all events with detailed information
   - Create new events with complete details (name, date, venue, capacity, pricing)
   - Delete events (system handles associated booking cleanup)
   - Modify event details including price adjustments

2. **Booking Oversight**
   - View all bookings across all events
   - Search for specific bookings by ID or username
   - Cancel bookings on behalf of customers
   - View booking analytics and statistics

3. **System Management**
   - Display seat maps showing occupied/available seats
   - View waiting queues for fully booked events
   - Access customer database
   - Generate reports and insights

### First Run
On first run, you'll need to register users. The system supports two types of users:
- **Customer**: Can browse events and make bookings
- **Admin**: Can manage events and view all bookings

### Customer Features
- Browse available events
- Book seats for concerts
- View your bookings
- Cancel bookings

### Admin Features
- Add new events with details (name, date, venue, total seats)
- Edit existing events
- Delete events
- View all bookings across all events

## Data Persistence

The application uses a file-based storage system for data persistence:

- **users.txt**: Stores user account information including usernames, passwords, and user types
- **events.txt**: Contains all event data (names, dates, venues, capacity, available seats, pricing)
- **bookings.txt**: Maintains booking records linking users to events with booking IDs

### Data Format
All data files use a structured text format that's human-readable and easy to parse. The files are automatically:
- Created on first save operation
- Updated whenever changes occur (new bookings, event modifications, etc.)
- Loaded on application startup to restore previous state

These files are excluded from version control via `.gitignore` to prevent committing user-generated data.

## Technical Details

### Programming Concepts Used
- **Modular Programming**: Code organized into separate modules for maintainability
- **File I/O**: Reading from and writing to text files for data persistence
- **Structs**: Custom data structures for Users, Events, and Bookings
- **Dynamic Arrays**: Managing collections of varying sizes
- **Input Validation**: Robust error checking for user inputs
- **Memory Management**: Proper allocation and deallocation of resources

### Compilation Flags
The Makefile uses:
- `-std=c99`: C99 standard compliance
- `-Wall -Wextra`: Enable comprehensive warnings
- `-O2`: Optimization level 2 for better performance

## Cleaning

To remove compiled files:

```bash
make clean
```

This removes all object files (*.o) and the executable.

## Future Enhancements

Potential improvements and features for future versions:
- Payment gateway integration
- Email confirmation system
- Seat selection (choose specific seats instead of auto-allocation)
- Multi-tier ticket pricing (VIP, Regular, Economy)
- Event categories and filtering
- User reviews and ratings
- Database backend (SQLite or MySQL) for better scalability
- Web or GUI interface

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is open source and available for educational purposes.
