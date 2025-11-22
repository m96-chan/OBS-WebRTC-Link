# OBS-WebRTC-Link Documentation

Welcome to the comprehensive documentation for the OBS-WebRTC-Link project. This documentation covers everything from getting started to advanced development topics.

---

## 📚 Documentation Index

### For Users

- **[Main README](../README.md)** - Project overview, installation, and usage
- **[Docker Setup Guide](../README.md#-development-environment-docker)** - Local LiveKit SFU testing environment

### For Developers

- **[Architecture Overview](ARCHITECTURE.md)** - System architecture, design principles, and component relationships
- **[API Reference](API-REFERENCE.md)** - Complete API documentation for all components
- **[Development Guide](DEVELOPMENT.md)** - Setting up development environment and workflow
- **[Testing Guide](TESTING.md)** - Writing and running tests
- **[Contributing Guidelines](CONTRIBUTING.md)** - Code style, formatting, and contribution workflow
- **[Project Structure](OBS-WebRTC-Link-Project-Structure.md)** - Directory structure and organization

---

## 🚀 Quick Start

### I want to use the plugin

1. Read the [Main README](../README.md) for installation and usage
2. Follow the [Usage Guide](../README.md#️-usage-guide) for configuration
3. Set up [Docker LiveKit](../README.md#-development-environment-docker) for local testing

### I want to contribute code

1. Read [Architecture Overview](ARCHITECTURE.md) to understand the system
2. Follow [Development Guide](DEVELOPMENT.md) to set up your environment
3. Review [Contributing Guidelines](CONTRIBUTING.md) for code style and workflow
4. Check [API Reference](API-REFERENCE.md) for component details
5. Read [Testing Guide](TESTING.md) to write tests

### I want to understand the codebase

1. Start with [Project Structure](OBS-WebRTC-Link-Project-Structure.md) to understand organization
2. Read [Architecture Overview](ARCHITECTURE.md) for design principles
3. Explore [API Reference](API-REFERENCE.md) for detailed component documentation
4. Review actual code in [src/](../src/) directory

---

## 📖 Documentation Structure

### [Architecture Overview](ARCHITECTURE.md)

**Topics covered**:
- High-level system architecture
- Design principles (separation of concerns, PIMPL, RAII)
- Component overview and responsibilities
- Data flow diagrams
- Protocol support (WHIP/WHEP/P2P)
- Threading model
- Error handling strategies

**Best for**: Understanding how the system works as a whole

### [API Reference](API-REFERENCE.md)

**Topics covered**:
- Core components: [PeerConnection](../src/core/peer-connection.hpp), [SignalingClient](../src/core/signaling-client.hpp), [WHIPClient](../src/core/whip-client.hpp), [WHEPClient](../src/core/whep-client.hpp), [ReconnectionManager](../src/core/reconnection-manager.hpp)
- Output components: [WebRTCOutput](../src/output/webrtc-output.hpp)
- Source components: [WebRTCSource](../src/source/webrtc-source.hpp)
- Data structures and enumerations
- Complete method signatures and descriptions
- Usage examples for each component

**Best for**: Looking up specific APIs and methods

### [Development Guide](DEVELOPMENT.md)

**Topics covered**:
- Development environment setup
- Building the project
- Development workflow
- Testing procedures
- Debugging techniques
- Code style guidelines
- Common development tasks
- Troubleshooting

**Best for**: Setting up and contributing to the project

### [Testing Guide](TESTING.md)

**Topics covered**:
- Test overview and organization
- Building and running tests
- Writing new tests
- Test patterns and best practices
- Coverage reports
- Continuous integration
- Troubleshooting test failures

**Best for**: Writing and maintaining tests

### [Contributing Guidelines](CONTRIBUTING.md)

**Topics covered**:
- Code formatting (clang-format, cmake-format)
- Editor integration (VS Code, CLion, Vim, Emacs)
- Pre-commit hooks
- Development workflow
- Pull request guidelines

**Best for**: Understanding contribution requirements

### [Project Structure](OBS-WebRTC-Link-Project-Structure.md)

**Topics covered**:
- Directory organization
- File naming conventions
- Separation of core, output, and source layers
- Dependency management
- Build system structure

**Best for**: Navigating the codebase

---

## 🔍 Finding Information

### By Topic

| Topic | Document | Section |
|-------|----------|---------|
| **Installation** | [README](../README.md) | Installation |
| **Usage** | [README](../README.md) | Usage Guide |
| **System Design** | [Architecture](ARCHITECTURE.md) | High-Level Architecture |
| **Core Components** | [API Reference](API-REFERENCE.md) | Core Components |
| **WebRTC Protocols** | [Architecture](ARCHITECTURE.md) | Protocol Support |
| **Threading** | [Architecture](ARCHITECTURE.md) | Threading Model |
| **Error Handling** | [Architecture](ARCHITECTURE.md) | Error Handling |
| **Building** | [Development](DEVELOPMENT.md) | Building the Project |
| **Testing** | [Testing](TESTING.md) | Running Tests |
| **Code Style** | [Contributing](CONTRIBUTING.md) | Code Style |
| **Debugging** | [Development](DEVELOPMENT.md) | Debugging |

### By Component

| Component | Header File | API Docs | Tests |
|-----------|-------------|----------|-------|
| **PeerConnection** | [peer-connection.hpp](../src/core/peer-connection.hpp) | [API Ref](API-REFERENCE.md#peerconnection) | [peer_connection_test.cpp](../tests/unit/peer_connection_test.cpp) |
| **SignalingClient** | [signaling-client.hpp](../src/core/signaling-client.hpp) | [API Ref](API-REFERENCE.md#signalingclient) | [signaling_client_test.cpp](../tests/unit/signaling_client_test.cpp) |
| **WHIPClient** | [whip-client.hpp](../src/core/whip-client.hpp) | [API Ref](API-REFERENCE.md#whipclient) | [whip_client_test.cpp](../tests/unit/whip_client_test.cpp) |
| **WHEPClient** | [whep-client.hpp](../src/core/whep-client.hpp) | [API Ref](API-REFERENCE.md#whepclient) | [whep_client_test.cpp](../tests/unit/whep_client_test.cpp) |
| **ReconnectionManager** | [reconnection-manager.hpp](../src/core/reconnection-manager.hpp) | [API Ref](API-REFERENCE.md#reconnectionmanager) | [reconnection_manager_test.cpp](../tests/unit/reconnection_manager_test.cpp) |
| **WebRTCOutput** | [webrtc-output.hpp](../src/output/webrtc-output.hpp) | [API Ref](API-REFERENCE.md#webrtcoutput) | [webrtc_output_test.cpp](../tests/unit/webrtc_output_test.cpp) |
| **WebRTCSource** | [webrtc-source.hpp](../src/source/webrtc-source.hpp) | [API Ref](API-REFERENCE.md#webrtcsource) | [webrtc_source_test.cpp](../tests/unit/webrtc_source_test.cpp) |

### By Task

| Task | Document | Section |
|------|----------|---------|
| **Clone repository** | [Development](DEVELOPMENT.md) | Clone Repository |
| **Install dependencies** | [Development](DEVELOPMENT.md) | Install Dependencies |
| **Configure CMake** | [Development](DEVELOPMENT.md) | Configure CMake |
| **Build project** | [Development](DEVELOPMENT.md) | Build |
| **Run tests** | [Testing](TESTING.md) | Running Tests |
| **Format code** | [Contributing](CONTRIBUTING.md) | Code Formatting |
| **Create pull request** | [Contributing](CONTRIBUTING.md) | Pull Request Guidelines |
| **Add new component** | [Development](DEVELOPMENT.md) | Add New Core Component |
| **Write tests** | [Testing](TESTING.md) | Writing Tests |
| **Debug issues** | [Development](DEVELOPMENT.md) | Debugging |

---

## 🎯 Common Use Cases

### Use Case 1: First-time Contributor

**Path**:
1. [Development Guide](DEVELOPMENT.md) - Set up environment
2. [Project Structure](OBS-WebRTC-Link-Project-Structure.md) - Understand organization
3. [Architecture Overview](ARCHITECTURE.md) - Learn system design
4. [Contributing Guidelines](CONTRIBUTING.md) - Learn workflow
5. Pick an issue and start coding!

### Use Case 2: Understanding a Specific Component

**Path**:
1. [API Reference](API-REFERENCE.md) - Look up component
2. Read header file in [src/](../src/)
3. Read implementation in [src/](../src/)
4. Review tests in [tests/unit/](../tests/unit/)

### Use Case 3: Debugging a Connection Issue

**Path**:
1. [Architecture Overview](ARCHITECTURE.md#protocol-support) - Understand protocol flow
2. [API Reference](API-REFERENCE.md#whipclient) - Check WHIP/WHEP client APIs
3. [Development Guide](DEVELOPMENT.md#debugging) - Enable verbose logging
4. [Testing Guide](TESTING.md#troubleshooting) - Check for known issues

### Use Case 4: Adding New Feature

**Path**:
1. [Architecture Overview](ARCHITECTURE.md) - Ensure design fits
2. [Development Guide](DEVELOPMENT.md#add-new-core-component) - Implementation steps
3. [Testing Guide](TESTING.md#writing-tests) - Write tests
4. [Contributing Guidelines](CONTRIBUTING.md) - Submit PR

---

## 📊 Documentation Statistics

- **Total Documentation Pages**: 7
- **API Components Documented**: 7
- **Code Examples**: 50+
- **Diagrams**: 5
- **Test Cases Documented**: 100+

---

## 🔄 Keeping Documentation Updated

Documentation is maintained alongside code changes:

- **When adding features**: Update [API Reference](API-REFERENCE.md) and [Architecture](ARCHITECTURE.md)
- **When changing build process**: Update [Development Guide](DEVELOPMENT.md)
- **When adding tests**: Update [Testing Guide](TESTING.md)
- **When changing code style**: Update [Contributing Guidelines](CONTRIBUTING.md)

All documentation is reviewed during pull request reviews.

---

## 📝 Documentation Conventions

### File Naming

- All documentation files use `.md` extension (Markdown)
- File names use kebab-case (e.g., `API-REFERENCE.md`)
- Main documentation index is `README.md`

### Formatting

- Headers use `#` for H1, `##` for H2, etc.
- Code blocks use triple backticks with language hint
- File paths use relative links
- Internal links use anchors for navigation

### Code Examples

All code examples are:
- Syntactically correct
- Compilable (when appropriate)
- Well-commented
- Follow project code style

---

## 🐛 Found Documentation Issues?

If you find errors, outdated information, or missing documentation:

1. Open an issue on [GitHub](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
2. Tag with `documentation` label
3. Or submit a pull request with fixes

---

## 📜 License

This documentation is part of the OBS-WebRTC-Link project and is licensed under **GPLv2**.

---

## 🙏 Acknowledgments

This documentation was created with contributions from:
- Project maintainers
- Community contributors
- Automated documentation tools

Special thanks to all contributors who help keep this documentation up-to-date and accurate.

---

**Last Updated**: 2025-01-22

**Documentation Version**: 0.1.0

---

## Additional Resources

### External Documentation

- [OBS Studio Plugin Development](https://obsproject.com/docs/plugins.html)
- [libdatachannel Documentation](https://github.com/paullouisageneau/libdatachannel)
- [WHIP Protocol Draft](https://datatracker.ietf.org/doc/draft-ietf-wish-whip/)
- [WHEP Protocol Draft](https://datatracker.ietf.org/doc/draft-murillo-whep/)
- [LiveKit Documentation](https://docs.livekit.io/)
- [WebRTC Specification](https://www.w3.org/TR/webrtc/)

### Community

- [GitHub Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
- [GitHub Discussions](https://github.com/m96-chan/OBS-WebRTC-Link/discussions)
- [Pull Requests](https://github.com/m96-chan/OBS-WebRTC-Link/pulls)

---

**Happy coding! 🚀**
