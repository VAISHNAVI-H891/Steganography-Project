# 🕵️‍♀️ LSB Image Steganography

This project implements **Least Significant Bit (LSB) Image Steganography**, a technique to hide secret text inside digital images without noticeable visual changes.  
It uses bit-level manipulation of pixel data to embed and extract messages.

## 📖 Overview

**Steganography** is the art of hiding information within another medium — in this case, an image.  
Using **Least Significant Bit (LSB)** substitution, the binary representation of text data is stored in the lowest bits of pixel values.

This ensures:
- The image looks unchanged to the human eye.
- The secret message can be perfectly retrieved later.

## ⚙️ Features

✅ Hide secret text inside an image  
✅ Extract hidden message from the image  
✅ Works with `.bmp` or `.png` files  
✅ Minimal change in image quality  
✅ Command-line interface for ease of use  

## 🧩 How It Works

### 🔹 Encoding Process:
1. Convert the secret message into binary form.
2. Replace the least significant bit (LSB) of each pixel with a bit from the message.
3. Save the modified image as the *stego image*.

### 🔹 Decoding Process:
1. Read the LSBs of pixels from the stego image.
2. Combine bits to form characters.
3. Stop when a delimiter (like `#`) is found, marking the end of the message.

## 🧱 Technologies Used

- **Language:** C  
- **Libraries:** `stdio.h`, `stdlib.h`, `string.h`, `math.h`  
- **Concepts:** Bitwise operations, File handling, Image I/O  

## 💡Outcome :

- 🔢 **Bitwise Operations:** Understanding how to manipulate bits for encoding and decoding data at the binary level.
- 🧮 **Image Representation:** Learned how image pixels are stored and processed in binary format.
- 🧰 **File Handling in C:** Reading, modifying, and writing binary files safely.
- 🧠 **Algorithm Design:** Designing step-by-step logic for encoding and decoding processes.
- 🧩 **Memory Management:** Allocating and freeing memory efficiently in C to handle image data.
- 🧵 **Debugging Skills:** Debugging segmentation faults and logical errors during image processing.
- 🧑‍💻 **Command-Line Interface Design:** Creating a user-friendly terminal-based tool.
- 🔐 **Data Security Concepts:** Understanding the basics of data hiding, confidentiality, and steganography techniques.
- 📚 **Documentation & Version Control:** Writing README files and managing source code with Git and GitHub.



