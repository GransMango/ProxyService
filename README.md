# Proxy Service: Project Overview

## Introduction

In this project, I developed a proxy server to facilitate communication between different devices using varying data formats. The assignment was inspired by IoT systems for home automation, where devices communicate via XML messages. However, some devices struggle with XML's overhead, so proxies are employed to convert data between efficient binary and human-readable XML formats.

## Task Breakdown

The project involved several key tasks:

1. **TCP Functionality**: I started by implementing TCP-related functions in the `connection.c` file. This allowed devices to establish connections and exchange data over the network.

2. **Proxy Logic**: I extended the `proxy.c` file to create the core proxy logic. This involved setting up an event loop to manage incoming connections from senders and receivers. I designed the loop to handle new connections, disconnect receivers, and process incoming messages.

3. **XML and Binary Conversion**: I implemented functions in `recordFromFormat.c` that created internal data structures (using `struct Record`) from input buffers containing XML or binary records. These functions streamlined the conversion process and improved code readability.

4. **Sender-Receiver Communication**: To ensure seamless communication, I made sure that the proxy could receive connections from both XML and binary senders. I established a system where records were correctly forwarded to the corresponding receivers based on the destination ID.

5. **Completing the Communication Flow**: I extended the proxy's capabilities to include communication between binary senders and receivers in both XML and binary modes. This comprehensive coverage allowed for versatile data exchange.

## Challenges and Solutions

While working on the project, I encountered a few challenges:

- **TCP Timeout Issue**: Initially, I struggled with implementing the timeout functionality for TCP connections. Despite trying various approaches, the `tcp_wait_timeout` function wouldn't work as expected. To work around this, I relied on Ctrl+C to manually exit the session, triggering the necessary cleanup code.

- **Binary Record Parsing**: Parsing binary records required understanding bitwise operations. After some research and experimentation, I managed to correctly interpret binary records and extract relevant information.

- **Design Choices**: One design choice involved whether to use `ntohl` for the ID and group fields. While the assignment explicitly mentioned using it for the username, it didn't specify for other fields. To maintain consistency, I decided to use `ntohl` for all relevant fields.

## Documentation and Design Choices

I documented my design choices in a PDF design document. This document covered three main areas: the TCP code in `connection.c`, the event loop design in `proxy.c`, and the XML-binary conversion process. I outlined successful implementations as well as any limitations or incomplete parts of the project.

## Conclusion

This project provided hands-on experience in developing a proxy server to bridge communication between devices using different data formats. By addressing challenges and making design decisions, I gained insights into networking concepts, data manipulation, and proxy functionality. The project not only enhanced my technical skills but also allowed me to build a functional system.
