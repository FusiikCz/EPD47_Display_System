/**
 * PC Server for E-Paper Display
 * 
 * This server acts as an intermediary between mobile devices and the ESP32 e-paper display.
 * It provides a web interface for phones to send text and images, and forwards the content
 * to the ESP32 device.
 */

//to-do
//So i noticed based on the print on the display we dont have the auto new line starting for example i tried putting lorem ipsum 
// and it just have one line instead of the whole few lines on one character there is space for like 48 characters. 
// We can make it 45 just to be sure and figure out the very last line where the commands are said like: "obrazovka vymazána"


const express = require('express');
const multer = require('multer');
const cors = require('cors');
const path = require('path');
const fs = require('fs');
const app = express();
const port = 3000;
const sharp = require('sharp'); // Add Sharp image processing library

// Configure storage for uploaded files
const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    const uploadDir = path.join(__dirname, 'uploads');
    if (!fs.existsSync(uploadDir)) {
      fs.mkdirSync(uploadDir, { recursive: true });
    }
    cb(null, uploadDir);
  },
  filename: function (req, file, cb) {
    cb(null, Date.now() + path.extname(file.originalname));
  }
});

const upload = multer({ storage: storage });

// ESP32 device configuration
let espDeviceIP = '';
let registeredDevices = [];

// Content queue for each device
let contentQueue = {};

// Device last fetch timestamps for throttling
let lastFetchTimestamps = {};

// Constants
const MAX_TEXT_LENGTH = 1000; // Maximum text length
const MIN_POLL_INTERVAL = 2000; // Minimum time between polls (2 seconds)
const DEVICE_TIMEOUT = 10 * 60 * 1000; // 10 minutes in milliseconds
const ALLOWED_IMAGE_TYPES = ['image/jpeg', 'image/png']; // Allowed image types
const EPD_WIDTH = 960; // E-paper display width
const EPD_HEIGHT = 540; // E-paper display height (including status area)
const MAIN_AREA_HEIGHT = 480; // Main display area height
const MAX_CHARS_PER_LINE = 45; // Maximum characters per line for text wrapping

// Device status tracking
let deviceStatus = {};

// Helper function to validate IP address
function isValidIPAddress(ip) {
  const ipRegex = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  return ipRegex.test(ip);
}

// Helper function to clean up uploaded files
function cleanupUploadedFile(filePath) {
  if (filePath && fs.existsSync(filePath)) {
    fs.unlink(filePath, (err) => {
      if (err) {
        console.error(`Error deleting file ${filePath}:`, err);
      } else {
        console.log(`Successfully deleted file: ${filePath}`);
      }
    });
  }
}

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));
app.use('/uploads', express.static(path.join(__dirname, 'uploads')));

// Routes
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Register ESP32 device
app.post('/register-device', (req, res) => {
  const { ip } = req.body;
  if (!ip) {
    return res.status(400).json({ error: 'IP address is required' });
  }
  
  if (!isValidIPAddress(ip)) {
    return res.status(400).json({ error: 'Invalid IP address format' });
  }
  
  if (!registeredDevices.includes(ip)) {
    registeredDevices.push(ip);
    contentQueue[ip] = [];
    lastFetchTimestamps[ip] = Date.now();
    deviceStatus[ip] = 'online';
    console.log(`ESP32 device registered with IP: ${ip}`);
  } else {
    // Update last fetch time and status for existing device
    lastFetchTimestamps[ip] = Date.now();
    deviceStatus[ip] = 'online';
  }
  
  res.json({ success: true, message: `Device registered with IP ${ip}` });
});

// Poll for content (used by ESP32 client)
app.post('/poll-content', (req, res) => {
  console.log(registeredDevices);
  const { ip } = req.body;
  if (!ip || !registeredDevices.includes(ip)) {
    return res.status(400).json({ error: 'Device not registered' });
  }
  
  // Update last fetch timestamp and status
  lastFetchTimestamps[ip] = Date.now();
  deviceStatus[ip] = 'online';
  
  // Implement throttling
  const now = Date.now();
  const lastFetch = lastFetchTimestamps[ip] || 0;
  
  // if (now - lastFetch < MIN_POLL_INTERVAL) {
  //   return res.status(429).json({ 
  //     error: 'Polling too frequently', 
  //     retryAfter: MIN_POLL_INTERVAL - (now - lastFetch) 
  //   });
  // }
  
  console.log(contentQueue[ip]);

  // Check if there's content in the queue for this device
  if (contentQueue[ip] && contentQueue[ip].length > 0) {
    const content = contentQueue[ip].shift();
    console.log("odesilame", content);
    return res.json(content);
  }
  
  // No content available
  return res.json({ type: 'none' });
});

// Get list of registered devices
app.get('/devices', (req, res) => {
  // Check for timed out devices
  const now = Date.now();
  registeredDevices.forEach(ip => {
    const lastFetch = lastFetchTimestamps[ip] || 0;
    if (now - lastFetch > DEVICE_TIMEOUT) {
      deviceStatus[ip] = 'offline';
    }
  });
  
  const devicesWithStatus = registeredDevices.map(ip => ({
    ip,
    active: ip === espDeviceIP,
    lastFetch: lastFetchTimestamps[ip] || 0,
    status: deviceStatus[ip] || 'unknown',
    lastSeen: lastFetchTimestamps[ip] ? new Date(lastFetchTimestamps[ip]).toLocaleString() : 'Never'
  }));
  
  res.json({ devices: devicesWithStatus });
});

// Set ESP32 IP address (for direct communication mode)
app.post('/set-device', (req, res) => {
  const { ip } = req.body;
  if (!ip) {
    return res.status(400).json({ error: 'IP address is required' });
  }
  
  if (!isValidIPAddress(ip)) {
    return res.status(400).json({ error: 'Invalid IP address format' });
  }
  
  espDeviceIP = ip;
  
  // Also register the device if not already registered
  if (!registeredDevices.includes(ip)) {
    registeredDevices.push(ip);
    contentQueue[ip] = [];
    lastFetchTimestamps[ip] = 0;
  }
  
  console.log(`ESP32 device IP set to: ${espDeviceIP}`);
  res.json({ success: true, message: `Device IP set to ${espDeviceIP}` });
});

// Helper function to wrap text at specified character count
function wrapText(text, maxCharsPerLine) {
  if (!text) return '';
  
  // First split by manual newlines
  const paragraphs = text.split('\n');
  const wrappedParagraphs = [];
  
  for (const paragraph of paragraphs) {
    if (paragraph.length <= maxCharsPerLine) {
      // Paragraph fits on one line
      wrappedParagraphs.push(paragraph);
    } else {
      // Need to wrap this paragraph
      let remainingText = paragraph;
      while (remainingText.length > 0) {
        if (remainingText.length <= maxCharsPerLine) {
          // Last piece fits on one line
          wrappedParagraphs.push(remainingText);
          break;
        }
        
        // Look for a space to break at
        let breakPos = maxCharsPerLine;
        while (breakPos > 0 && remainingText.charAt(breakPos) !== ' ') {
          breakPos--;
        }
        
        // If no space found, force break at maxCharsPerLine
        if (breakPos === 0) {
          breakPos = maxCharsPerLine;
        }
        
        wrappedParagraphs.push(remainingText.substring(0, breakPos));
        // Skip the space when starting the next line
        remainingText = remainingText.substring(breakPos + 1);
      }
    }
  }
  
  return wrappedParagraphs.join('\n');
}

// Send text to ESP32
app.post('/send-text', async (req, res) => {
  const { text, deviceIp, textSize } = req.body;
  
  if (!text) {
    return res.status(400).json({ error: 'Text is required' });
  }
  
  // Validate text length
  if (text.length > MAX_TEXT_LENGTH) {
    return res.status(400).json({ 
      error: `Text is too long. Maximum length is ${MAX_TEXT_LENGTH} characters.` 
    });
  }
  
  // Determine which device to send to
  const targetIp = deviceIp || espDeviceIP;
  
  if (!targetIp) {
    return res.status(400).json({ error: 'No device IP specified or set' });
  }
  
  if (!isValidIPAddress(targetIp)) {
    return res.status(400).json({ error: 'Invalid IP address format' });
  }
  
  // Apply text wrapping based on text size
  let maxChars = MAX_CHARS_PER_LINE; // Default for medium text
  if (textSize === 'small') {
    maxChars = 55; // More characters fit on a line with smaller font
  } else if (textSize === 'large') {
    maxChars = 35; // Fewer characters fit on a line with larger font
  }
  
  // Wrap the text before sending
  const wrappedText = wrapText(text, maxChars);
  console.log(`Original text length: ${text.length}, wrapped text length: ${wrappedText.length}`);
  
  // Add to content queue if device is registered
  if (registeredDevices.includes(targetIp)) {
    if (!contentQueue[targetIp]) {
      contentQueue[targetIp] = [];
    }
    
    contentQueue[targetIp].push({
      type: 'text',
      data: wrappedText,
      textSize: textSize || 'medium' // Default to medium if not specified
    });
    
    console.log(`Text added to queue for device ${targetIp} with size: ${textSize || 'medium'}`);
    return res.json({ success: true, message: 'Text sent to device' });
  }
  
  return res.status(404).json({ error: 'Device not registered' });
});

// Send image to ESP32
app.post('/send-image', upload.single('image'), async (req, res) => {
  if (!req.file) {
    return res.status(400).json({ error: 'Image is required' });
  }
  
  // Validate image type
  if (!ALLOWED_IMAGE_TYPES.includes(req.file.mimetype)) {
    // Delete the invalid file
    cleanupUploadedFile(req.file.path);
    return res.status(400).json({ 
      error: `Invalid image type. Allowed types: ${ALLOWED_IMAGE_TYPES.join(', ')}` 
    });
  }
  
  // Determine which device to send to
  const targetIp = req.body.deviceIp || espDeviceIP;
  
  if (!targetIp) {
    // Delete the file if no device is specified
    cleanupUploadedFile(req.file.path);
    return res.status(400).json({ error: 'No device IP specified or set' });
  }
  
  if (!isValidIPAddress(targetIp)) {
    // Delete the file if IP is invalid
    cleanupUploadedFile(req.file.path);
    return res.status(400).json({ error: 'Invalid IP address format' });
  }
  
  try {
    console.log(`Processing image: ${req.file.path}`);
    // Process the image for e-paper display
    const imagePath = req.file.path;
    
    // Get image info before processing
    const imageInfo = await sharp(imagePath).metadata();
    console.log(`Original image: ${imageInfo.width}x${imageInfo.height}, format: ${imageInfo.format}`);
    
    const processedImageBase64 = await processImageForEPD(imagePath);
    console.log(`Processed image base64 length: ${processedImageBase64.length}`);
    
    // Add to content queue if device is registered
    if (registeredDevices.includes(targetIp)) {
      if (!contentQueue[targetIp]) {
        contentQueue[targetIp] = [];
      }
      
      contentQueue[targetIp].push({
        type: 'processed_image',  // Changed type to indicate pre-processed image
        data: processedImageBase64
      });
      
      console.log(`Processed image added to queue for device ${targetIp}`);
      
      // Clean up the file after processing
      cleanupUploadedFile(imagePath);
      
      return res.json({ success: true, message: 'Image processed and sent to device' });
    }
    
    // Clean up the file if device is not registered
    cleanupUploadedFile(imagePath);
    return res.status(404).json({ error: 'Device not registered' });
  } catch (error) {
    console.error('Error in /send-image:', error);
    // Clean up the file if processing fails
    cleanupUploadedFile(req.file.path);
    return res.status(500).json({ error: 'Error processing image: ' + error.message });
  }
});

// Clear display
app.post('/clear-display', async (req, res) => {
  // Determine which device to send to
  const targetIp = req.body.deviceIp || espDeviceIP;
  
  if (!targetIp) {
    return res.status(400).json({ error: 'No device IP specified or set' });
  }
  
  if (!isValidIPAddress(targetIp)) {
    return res.status(400).json({ error: 'Invalid IP address format' });
  }
  
  // Add to content queue if device is registered
  if (registeredDevices.includes(targetIp)) {
    if (!contentQueue[targetIp]) {
      contentQueue[targetIp] = [];
    }
    
    contentQueue[targetIp].push({
      type: 'clear'
    });
    
    console.log(`Clear command added to queue for device ${targetIp}`);
    return res.json({ success: true, message: 'Clear command sent to device' });
  }
  
  return res.status(404).json({ error: 'Device not registered' });
});

// Cleanup old uploads (run every hour)
setInterval(() => {
  const uploadDir = path.join(__dirname, 'uploads');
  if (fs.existsSync(uploadDir)) {
    fs.readdir(uploadDir, (err, files) => {
      if (err) {
        console.error('Error reading uploads directory:', err);
        return;
      }
      
      const now = Date.now();
      files.forEach(file => {
        const filePath = path.join(uploadDir, file);
        fs.stat(filePath, (err, stats) => {
          if (err) {
            console.error(`Error getting stats for file ${filePath}:`, err);
            return;
          }
          
          // Delete files older than 1 hour
          if (now - stats.mtime.getTime() > 3600000) {
            cleanupUploadedFile(filePath);
          }
        });
      });
    });
  }
}, 3600000); // Run every hour

// Start the server
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
  
  // Create uploads directory if it doesn't exist
  const uploadDir = path.join(__dirname, 'uploads');
  if (!fs.existsSync(uploadDir)) {
    fs.mkdirSync(uploadDir, { recursive: true });
  }
});

// Periodic check for offline devices (runs every minute)
setInterval(() => {
  const now = Date.now();
  registeredDevices.forEach(ip => {
    const lastFetch = lastFetchTimestamps[ip] || 0;
    if (now - lastFetch > DEVICE_TIMEOUT) {
      deviceStatus[ip] = 'offline';
      console.log(`Device ${ip} marked as offline (no activity for ${DEVICE_TIMEOUT/60000} minutes)`);
    }
  });
}, 60000); // Check every minute

// Get the content queue for a specific device
app.get('/device-queue', (req, res) => {
  const { ip } = req.query;
  
  if (!ip) {
    return res.status(400).json({ error: 'IP address is required' });
  }
  
  if (!registeredDevices.includes(ip)) {
    return res.status(404).json({ error: 'Device not registered' });
  }
  
  const queue = contentQueue[ip] || [];
  
  res.json({ queue });
});

const dgram = require('dgram');
const udpServer = dgram.createSocket('udp4');

let deviceLastSeen = {};

const BROADCAST_PORT = 4210; 
const BROADCAST_MESSAGE = Buffer.from('who-is-epd'); 
const BROADCAST_INTERVAL = 10000; // každých 10 sekund 

// Odeslat broadcast zprávu 
setInterval(() => { 
  udpServer.setBroadcast(true); 
  udpServer.send(BROADCAST_MESSAGE, 0, BROADCAST_MESSAGE.length, BROADCAST_PORT, '255.255.255.255', (err) => { 
    if (err) console.error('UDP broadcast error:', err); 
    else console.log('Broadcast message sent: who-is-epd'); 
  }); 
}, BROADCAST_INTERVAL);

app.post('/heartbeat', (req, res) => {
  const { ip } = req.body;
  if (ip && registeredDevices.includes(ip)) {
    deviceLastSeen[ip] = Date.now();
  }
  res.json({ success: true });
});

udpServer.on('message', (msg, rinfo) => {
  if (msg.toString() === 'epd-online') {
    const ip = rinfo.address;
    if (!registeredDevices.includes(ip)) {
      registeredDevices.push(ip);
      contentQueue[ip] = [];
      lastFetchTimestamps[ip] = 0;
      console.log(`Discovered and registered via UDP: ${ip}`);
    }
    deviceLastSeen[ip] = Date.now();
  }
});

udpServer.bind(4210);

app.get('/devices', (req, res) => {
  const now = Date.now();
  const devicesWithStatus = registeredDevices.map(ip => ({
    ip,
    active: ip === espDeviceIP,
    online: (deviceLastSeen[ip] && now - deviceLastSeen[ip] < 20000)
  }));
  res.json({ devices: devicesWithStatus });
});

// Helper function to process image for e-paper display
async function processImageForEPD(imagePath) {
  try {
    // Validate the image file exists
    if (!fs.existsSync(imagePath)) {
      throw new Error(`Image file not found: ${imagePath}`);
    }

    // Get original image metadata with better error handling
    let metadata;
    try {
      metadata = await sharp(imagePath).metadata();
    } catch (err) {
      console.error(`Error reading image metadata: ${err.message}`);
      throw new Error(`Unsupported image format. Please use JPEG or PNG files.`);
    }
    
    console.log(`Original image: ${metadata.width}x${metadata.height}, format: ${metadata.format}`);
    
    const EPD_WIDTH = 960;
    const MAIN_AREA_HEIGHT = 480;
    
    // Use a direct approach that fills the screen while maintaining aspect ratio
    // This will crop the image if necessary to avoid white space
    const processedImageBuffer = await sharp(imagePath)
      // First resize to cover the entire display area (may crop parts of the image)
      .resize({
        width: EPD_WIDTH,
        height: MAIN_AREA_HEIGHT,
        fit: 'fill',  // This ensures the image fills the entire area with no white space
        position: 'center'  // Center the image to keep the main subject visible
      })
      .grayscale()
      .normalize()
      // Use Floyd-Steinberg dithering for better appearance on e-paper
      .threshold(128, { dither: 'FloydSteinberg' })
      .raw()
      .toBuffer();
    
    console.log(`Processed image size: ${processedImageBuffer.length} bytes`);
    
    // Convert to base64 for transmission
    return processedImageBuffer.toString('base64');
  } catch (error) {
    console.error('Error processing image:', error);
    throw error;
  }
}