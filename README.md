# Functional Multithreaded Operating System Kernel for RISC-V

## Project Description

This project involves the development of a functional operating system kernel supporting multithreading with time-sharing, memory allocation, thread management, semaphores, and asynchronous context switching. The kernel will be implemented in C/C++ for the RISC-V architecture and run in a virtual environment.

## Features

- **Thread Management:** Create, manage, and terminate threads.
- **Memory Allocation:** Efficiently allocate and deallocate memory blocks.
- **Semaphores:** Provide synchronization mechanisms using semaphores.
- **Time-Sharing:** Implement time-sharing for thread execution.
- **Context Switching:** Support asynchronous context switching triggered by timer and keyboard interrupts.
- **Embedded System Simulation:** Kernel and application share the same address space, typical of embedded systems.

## Project Structure

- **Kernel:** Core functionalities of the operating system, implemented in C++ with assembly as needed.
- **User Application:** Test cases and user programs to interact with the kernel.
- **Libraries:**
  - `app.lib`: Contains the user application code.
  - `hw.lib`: Provides hardware access modules for the virtual environment.

## Setup and Installation

1. **Prerequisites:**
   - RISC-V toolchain
   - Emulator for RISC-V (e.g., QEMU)
   - xv6 educational operating system (modified version)

2. **Building the Project:**
   ```bash
   make all
   ```

3. **Running the Project:**
   ```bash
   make run
   ```

## Usage

- **Main Entry Point:**
  The user application should define a `void userMain();` function which will be executed by the kernel.
- **Kernel API:**
  - **Memory Allocation:** 
    - `void* mem_alloc(size_t size);`
    - `int mem_free(void* ptr);`
  - **Thread Management:** 
    - `int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg);`
    - `int thread_exit(void* retval);`
    - `int thread_join(thread_t thread, void** retval);`
    - `int thread_dispatch();`
  - **Semaphore Operations:** 
    - `int sem_open(sem_t* handle, unsigned init);`
    - `int sem_close(sem_t handle);`
    - `int sem_wait(sem_t id);`
    - `int sem_signal(sem_t id);`

## Contribution Guidelines

- Fork the repository.
- Create a new branch for your feature or bugfix.
- Submit a pull request with detailed descriptions of your changes.

## License

This project is licensed under the MIT License. See the `LICENSE` file for more details.

## Acknowledgements

- Prof. Dr. Dragan Milićev
- School of Electrical Engineering, University of Belgrade
- xv6 Educational Operating System

## Contact

For any questions or inquiries, please contact vukasinjovanovic2@gmail.com

---
