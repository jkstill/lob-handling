package main

import (
	"fmt"
	//"io/ioutil"
	"os"
)

const (
	INPUT_FILE  = "kitty.hex"
	OUTPUT_FILE = "kitty-Go.jpg"
)

// Function to convert a single hex character to its numerical value
func hexCharToValue(c byte) (byte, error) {
	switch {
	case c >= '0' && c <= '9':
		return c - '0', nil
	case c >= 'a' && c <= 'f':
		return c - 'a' + 10, nil
	case c >= 'A' && c <= 'F':
		return c - 'A' + 10, nil
	default:
		return 0, fmt.Errorf("invalid hex character: %c", c)
	}
}

// Function to convert hex string to binary data
func hexToBinary(hexData string) ([]byte, error) {
	hexLength := len(hexData)
	if hexLength%2 != 0 {
		return nil, fmt.Errorf("hex data length is not even")
	}

	binaryData := make([]byte, hexLength/2)

	for i := 0; i < hexLength/2; i++ {
		highNibble, err := hexCharToValue(hexData[2*i])
		if err != nil {
			return nil, err
		}
		lowNibble, err := hexCharToValue(hexData[2*i+1])
		if err != nil {
			return nil, err
		}
		binaryData[i] = (highNibble << 4) | lowNibble
	}

	return binaryData, nil
}

func main() {
	// Read the hex data from the input file
	hexDataBytes, err := os.ReadFile(INPUT_FILE)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot open input file: %v\n", err)
		os.Exit(1)
	}
	hexData := string(hexDataBytes)

	// Check if the hex data length is even
	hexLength := len(hexData)
	if hexLength%2 != 0 {
		fmt.Fprintf(os.Stderr, "Hex data length is not even.\n")
		os.Exit(1)
	}

	// Allocate buffer for binary data
	var binaryData []byte

	// Convert hex to binary 10,240 times
	for i := 0; i < 10240; i++ {
		binaryData, err = hexToBinary(hexData)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error converting hex to binary: %v\n", err)
			os.Exit(1)
		}
	}

	// Write binary data to the output file
	err = os.WriteFile(OUTPUT_FILE, binaryData, 0644)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot write output file: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("Conversion completed successfully.")
}

