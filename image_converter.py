#!/usr/bin/env python3
"""
Usage:
    python image_converter.py --gui
"""

import os
import sys
import argparse
from PIL import Image
import numpy as np
import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog

# Display dimensions
EPD_WIDTH = 960
EPD_HEIGHT = 540

def convert_image_to_4bit_grayscale(input_path, output_path):
    """
    Convert an image to 4-bit grayscale raw format for EPD47
    """
    try:
        # Open and convert image
        img = Image.open(input_path)
        
        # Convert to grayscale
        img = img.convert('L')
        
        # Resize to display dimensions while maintaining aspect ratio
        img.thumbnail((EPD_WIDTH, EPD_HEIGHT), Image.Resampling.LANCZOS)
        
        # Create a new image with exact display dimensions (black background)
        final_img = Image.new('L', (EPD_WIDTH, EPD_HEIGHT), 0)
        
        # Center the resized image
        x_offset = (EPD_WIDTH - img.width) // 2
        y_offset = (EPD_HEIGHT - img.height) // 2
        final_img.paste(img, (x_offset, y_offset))
        
        # Convert to numpy array
        img_array = np.array(final_img)
        
        # Convert 8-bit grayscale to 4-bit (0-15 range)
        img_4bit = (img_array >> 4).astype(np.uint8)
        
        # Pack two 4-bit pixels into one byte
        height, width = img_4bit.shape
        packed_data = np.zeros((height, width // 2), dtype=np.uint8)
        
        for y in range(height):
            for x in range(0, width, 2):
                pixel1 = img_4bit[y, x]
                pixel2 = img_4bit[y, x + 1] if x + 1 < width else 0
                packed_data[y, x // 2] = (pixel1 << 4) | pixel2
        
        # Save as raw binary file
        with open(output_path, 'wb') as f:
            f.write(packed_data.tobytes())
        
        print(f"Converted: {input_path} -> {output_path}")
        print(f"Output size: {len(packed_data.tobytes())} bytes")
        return True
        
    except Exception as e:
        print(f"Error converting {input_path}: {str(e)}")
        return False

def convert_to_bitmap(input_path, output_path):
    """
    Convert an image to bitmap format for SD card storage
    """
    try:
        # Open and convert image
        img = Image.open(input_path)
        
        # Convert to grayscale
        img = img.convert('L')
        
        # Resize to display dimensions
        img.thumbnail((EPD_WIDTH, EPD_HEIGHT), Image.Resampling.LANCZOS)
        
        # Create a new image with exact display dimensions
        final_img = Image.new('L', (EPD_WIDTH, EPD_HEIGHT), 255)
        
        # Center the resized image
        x_offset = (EPD_WIDTH - img.width) // 2
        y_offset = (EPD_HEIGHT - img.height) // 2
        final_img.paste(img, (x_offset, y_offset))
        
        # Save as bitmap
        final_img.save(output_path, 'BMP')
        
        print(f"Converted to bitmap: {input_path} -> {output_path}")
        return True
        
    except Exception as e:
        print(f"Error converting {input_path}: {str(e)}")
        return False

def batch_convert(input_folder, output_folder, format_type='raw'):
    """
    Batch convert all images in a folder
    """
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    supported_formats = ['.jpg', '.jpeg', '.png', '.bmp', '.tiff', '.gif']
    converted_count = 0
    
    for filename in os.listdir(input_folder):
        if any(filename.lower().endswith(ext) for ext in supported_formats):
            input_path = os.path.join(input_folder, filename)
            
            # Change extension based on format type
            base_name = os.path.splitext(filename)[0]
            if format_type == 'raw':
                output_filename = f"{base_name}.raw"
                output_path = os.path.join(output_folder, output_filename)
                success = convert_image_to_4bit_grayscale(input_path, output_path)
            else:  # bitmap
                output_filename = f"{base_name}.bmp"
                output_path = os.path.join(output_folder, output_filename)
                success = convert_to_bitmap(input_path, output_path)
            
            if success:
                converted_count += 1
    
    print(f"\nBatch conversion complete: {converted_count} images converted")

def gui_batch_convert():
    root = tk.Tk()
    root.withdraw()
    messagebox.showinfo("Image Converter", "Select one or more image files to convert.")
    file_paths = filedialog.askopenfilenames(
        title="Select Images",
        filetypes=[("Image Files", ".jpg .jpeg .png .bmp .tiff .gif")]
    )
    if not file_paths:
        print("No files selected.")
        return
    output_folder = filedialog.askdirectory(title="Select Output Folder")
    if not output_folder:
        print("No output folder selected.")
        return
    format_type = simpledialog.askstring(
        "Output Format",
        "Enter output format (raw or bmp):",
        initialvalue="raw"
    )
    if not format_type or format_type not in ("raw", "bmp"):
        messagebox.showerror("Error", "Invalid format. Please enter 'raw' or 'bmp'.")
        return
    converted_count = 0
    for input_path in file_paths:
        base_name = os.path.splitext(os.path.basename(input_path))[0]
        if format_type == 'raw':
            output_filename = f"{base_name}.raw"
            output_path = os.path.join(output_folder, output_filename)
            success = convert_image_to_4bit_grayscale(input_path, output_path)
        else:
            output_filename = f"{base_name}.bmp"
            output_path = os.path.join(output_folder, output_filename)
            success = convert_to_bitmap(input_path, output_path)
        if success:
            converted_count += 1
    messagebox.showinfo("Conversion Complete", f"{converted_count} images converted.")
    print(f"\nGUI batch conversion complete: {converted_count} images converted")

def launch_full_gui():
    class ImageConverterGUI:
        def __init__(self, master):
            self.master = master
            master.title("Image Converter for LilyGo EPD47")
            master.geometry("500x350")
            self.mode = tk.StringVar(value="single")
            self.format_type = tk.StringVar(value="raw")
            self.input_path = tk.StringVar()
            self.output_path = tk.StringVar()
            self.status = tk.StringVar()

            # Mode selection
            tk.Label(master, text="Mode:").pack(anchor="w", padx=10, pady=(10,0))
            mode_frame = tk.Frame(master)
            mode_frame.pack(anchor="w", padx=20)
            tk.Radiobutton(mode_frame, text="Single Image", variable=self.mode, value="single", command=self.update_mode).pack(side="left")
            tk.Radiobutton(mode_frame, text="Batch Folder", variable=self.mode, value="batch", command=self.update_mode).pack(side="left")

            # Input selection
            self.input_label = tk.Label(master, text="Input Image:")
            self.input_label.pack(anchor="w", padx=10, pady=(10,0))
            input_frame = tk.Frame(master)
            input_frame.pack(fill="x", padx=20)
            self.input_entry = tk.Entry(input_frame, textvariable=self.input_path, width=40)
            self.input_entry.pack(side="left", fill="x", expand=True)
            self.input_button = tk.Button(input_frame, text="Browse", command=self.browse_input)
            self.input_button.pack(side="left", padx=5)

            # Output selection
            self.output_label = tk.Label(master, text="Output File:")
            self.output_label.pack(anchor="w", padx=10, pady=(10,0))
            output_frame = tk.Frame(master)
            output_frame.pack(fill="x", padx=20)
            self.output_entry = tk.Entry(output_frame, textvariable=self.output_path, width=40)
            self.output_entry.pack(side="left", fill="x", expand=True)
            self.output_button = tk.Button(output_frame, text="Browse", command=self.browse_output)
            self.output_button.pack(side="left", padx=5)

            # Format selection
            tk.Label(master, text="Output Format:").pack(anchor="w", padx=10, pady=(10,0))
            format_frame = tk.Frame(master)
            format_frame.pack(anchor="w", padx=20)
            tk.Radiobutton(format_frame, text="4-bit RAW", variable=self.format_type, value="raw").pack(side="left")
            tk.Radiobutton(format_frame, text="Bitmap (BMP)", variable=self.format_type, value="bmp").pack(side="left")

            # Convert button
            self.convert_button = tk.Button(master, text="Convert", command=self.convert)
            self.convert_button.pack(pady=15)

            # Status label
            self.status_label = tk.Label(master, textvariable=self.status, fg="blue")
            self.status_label.pack(pady=(0,10))

            self.update_mode()

        def update_mode(self):
            if self.mode.get() == "single":
                self.input_label.config(text="Input Image:")
                self.output_label.config(text="Output File:")
            else:
                self.input_label.config(text="Input Folder:")
                self.output_label.config(text="Output Folder:")

        def browse_input(self):
            if self.mode.get() == "single":
                path = filedialog.askopenfilename(title="Select Image", filetypes=[("Image Files", ".jpg .jpeg .png .bmp .tiff .gif")])
            else:
                path = filedialog.askdirectory(title="Select Input Folder")
            if path:
                self.input_path.set(path)

        def browse_output(self):
            if self.mode.get() == "single":
                path = filedialog.asksaveasfilename(title="Select Output File", defaultextension=f".{self.format_type.get()}", filetypes=[("RAW", ".raw"), ("BMP", ".bmp"), ("All Files", ".*")])
            else:
                path = filedialog.askdirectory(title="Select Output Folder")
            if path:
                self.output_path.set(path)

        def convert(self):
            self.status.set("")
            input_path = self.input_path.get()
            output_path = self.output_path.get()
            fmt = self.format_type.get()
            if not input_path or not output_path:
                messagebox.showerror("Error", "Please select input and output paths.")
                return
            try:
                if self.mode.get() == "single":
                    if not os.path.isfile(input_path):
                        messagebox.showerror("Error", "Input file does not exist.")
                        return
                    if fmt == "raw":
                        success = convert_image_to_4bit_grayscale(input_path, output_path)
                    else:
                        success = convert_to_bitmap(input_path, output_path)
                    if success:
                        self.status.set("Conversion successful!")
                        messagebox.showinfo("Success", "Image converted successfully.")
                    else:
                        self.status.set("Conversion failed.")
                else:
                    if not os.path.isdir(input_path):
                        messagebox.showerror("Error", "Input folder does not exist.")
                        return
                    batch_convert(input_path, output_path, fmt)
                    self.status.set("Batch conversion complete!")
                    messagebox.showinfo("Success", "Batch conversion complete!")
            except Exception as e:
                self.status.set(f"Error: {str(e)}")
                messagebox.showerror("Error", str(e))

    root = tk.Tk()
    app = ImageConverterGUI(root)
    root.mainloop()

def main():
    parser = argparse.ArgumentParser(description='Convert images for LilyGo EPD47 display')
    parser.add_argument('--batch', action='store_true', help='Batch convert all images in a folder')
    parser.add_argument('--format', choices=['raw', 'bmp'], default='raw', help='Output format (raw or bmp)')
    parser.add_argument('--gui', action='store_true', help='Launch GUI wizard for multi-file selection and conversion')
    parser.add_argument('--full-gui', action='store_true', help='Launch full graphical user interface')
    parser.add_argument('input', nargs='?', help='Input image file or folder')
    parser.add_argument('output', nargs='?', help='Output file or folder')
    
    args = parser.parse_args()
    
    if args.full_gui:
        launch_full_gui()
        return
    
    if args.gui:
        gui_batch_convert()
        return
    
    if args.batch:
        if not args.input or not args.output:
            print("Error: Input and output folders must be specified for batch conversion")
            sys.exit(1)
        if not os.path.isdir(args.input):
            print("Error: Input must be a directory for batch conversion")
            sys.exit(1)
        batch_convert(args.input, args.output, args.format)
    else:
        if not args.input or not args.output:
            print("Error: Input file and output file must be specified")
            sys.exit(1)
        if not os.path.isfile(args.input):
            print("Error: Input file does not exist")
            sys.exit(1)
        
        if args.format == 'raw':
            success = convert_image_to_4bit_grayscale(args.input, args.output)
        else:
            success = convert_to_bitmap(args.input, args.output)
        
        if not success:
            sys.exit(1)

if __name__ == '__main__':
    main()