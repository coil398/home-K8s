# Use a base image with C++ support for the build stage
FROM gcc:latest AS build

# Set the working directory
WORKDIR /usr/src/app

# Copy the source code
COPY src/echo-server .

# Build the echo server
RUN g++ -o echo-server main.cpp -lboost_system -lboost_thread -lpthread

# Use a distroless base image for the final stage
FROM gcr.io/distroless/cc

# Copy the built echo server binary to the final stage
COPY --from=build /usr/src/app/echo-server /usr/local/bin/echo-server

# Expose port 8080
EXPOSE 8080

# Run the echo server
CMD ["echo-server"]
