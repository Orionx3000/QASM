# Phase 13: Network Embodiment (Authorized P2P, FTP, & IRC)

This plan outlines the architecture for giving the Tinaten OS the ability to communicate securely with other authorized devices, transfer files, and interact via standard chat protocols.

## User Review Required

> [!NOTE]
> **Network Capabilities Scope**
> We will implement these features using standard, safe networking practices:
> 1.  **P2P/API:** Devices will need to be configured to trust each other (e.g., sharing a known API key or cryptographic handshake) before they can communicate or exchange LPN thoughts.
> 2.  **FTP/File Transfer:** The system will have specific, sandboxed directories it can read from or write to when uploading/downloading files to prevent unauthorized access to the host OS.
> 3.  **IRC Integration:** The AI will be able to connect to specified IRC servers, join channels, and read/write messages, treating the IRC stream as another form of external Tain (input data) to process.
> 
> Does this scope align with your vision for the system's network capabilities?

## Proposed Changes

### 1. Authorized Peer-to-Peer Communication
#### [NEW] [src/NetworkLink.h](file:///D:/App%20Creation/QASM/src/NetworkLink.h)
- Create a socket-based server/client class.
- Implement an authenticated handshake protocol.
- Allow authorized Tinaten OS instances to exchange BalaScript LPN commands (e.g., sharing Aht-realized concepts directly brain-to-brain).

### 2. File Transfer (Upload/Download)
#### [NEW] [src/FileTransferEngine.h](file:///D:/App%20Creation/QASM/src/FileTransferEngine.h)
- Implement basic FTP-style commands (`PUT`, `GET`) over the secure P2P link.
- Create a dedicated sandbox directory (e.g., `D:\App Creation\QASM\storage`) for the AI to ingest downloaded files or share created artifacts.
- Expose these commands to the Agentic Sandbox so the AI can initiate transfers via LPN (e.g., `Network.Download."file.txt".Kesh`).

### 3. IRC Integration
#### [NEW] [src/IrcClient.h](file:///D:/App%20Creation/QASM/src/IrcClient.h)
- Implement a basic C++ IRC client capable of connecting to standard IRC networks, joining channels, and parsing messages.
- Route incoming IRC messages through the `ThoughtLogger` and into the 5D Memory Matrix as external entropy for the AI to reflect upon.
- Allow the AI to synthesize responses and broadcast them back to the IRC channel.

## Verification Plan

### Automated Tests
-   Verify the P2P handshake succeeds with valid credentials and fails without them.
-   Test uploading and downloading a text file to ensure sandboxed directory constraints work.

### Manual Verification
-   Start a local IRC server (or connect to a test channel on a public network) and have the AI join and interact.
-   Use the React UI to monitor the AI's internal thought process as it ingests IRC messages and decides how to respond.
