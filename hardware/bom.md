# Bill of Materials (BOM)

## ESP32 Wildlife Camera - Solar Powered System

### Core Components

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **ESP32-S3-DevKitC-1** | ESP32-S3 development board with PSRAM | 1 | $15.00 | Espressif/ESP32-S3-DevKitC-1 | Main controller with WiFi/BT |
| **OV2640 Camera Module** | 2MP camera module for ESP32 | 1 | $8.00 | Various/OV2640 | Wildlife photography optimized |
| **SX1276 LoRa Module** | 915MHz LoRa transceiver | 1 | $12.00 | HopeRF/RFM95W | Long-range communication |
| **18650 Li-ion Battery** | 3.7V 3000mAh rechargeable battery | 1 | $6.00 | Samsung INR18650-30Q | High capacity, protected |
| **18650 Battery Holder** | Single cell battery holder with wires | 1 | $2.00 | Keystone/1042 | Secure mounting |

### Solar Power System

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **Solar Panel** | 5V 2W monocrystalline panel | 1 | $10.00 | Various/110x60mm | From path light or standalone |
| **TP4056 Charging Module** | Li-ion battery charging IC with protection | 1 | $1.50 | Various/TP4056-1A | Micro USB input, protection |
| **Schottky Diode** | 1N5819 40V 1A diode | 1 | $0.25 | Fairchild/1N5819 | Reverse current protection |
| **100µF Capacitor** | Electrolytic capacitor 25V | 2 | $0.50 | Various/ECA-1EM101 | Power smoothing |
| **10kΩ Resistor** | 1/4W metal film resistor | 4 | $0.20 | Various | Voltage dividers |

### Sensors & Interface

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **PIR Motion Sensor** | HC-SR501 PIR motion detector | 1 | $2.00 | Various/HC-SR501 | Adjustable sensitivity |
| **BME280 Sensor** | Temperature, humidity, pressure | 1 | $5.00 | Bosch/BME280 | Weather monitoring (optional) |
| **MicroSD Card** | 32GB Class 10 microSD | 1 | $8.00 | SanDisk/Ultra 32GB | Image storage |
| **Status LED** | 3mm red LED | 2 | $0.50 | Various | Charging/status indicators |
| **220Ω Resistor** | Current limiting resistor | 2 | $0.10 | Various | LED current limiting |

### Enclosure & Mounting

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **Weatherproof Enclosure** | IP65 ABS plastic box | 1 | $12.00 | Hammond/1591XXFLBK | 3D printable alternative |
| **Cable Glands** | PG7 waterproof cable entry | 2 | $2.00 | Various/PG7 | Solar panel and antenna |
| **Silica Gel Packets** | Desiccant moisture absorber | 2 | $1.00 | Various | Humidity control |
| **Mounting Bracket** | Stainless steel L-bracket | 1 | $3.00 | Various | Tree/post mounting |
| **Security Screws** | Tamper-resistant screws | 4 | $2.00 | Various | Anti-theft mounting |

### Connectivity & Accessories

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **LoRa Antenna** | 915MHz 3dBi antenna with SMA | 1 | $8.00 | Various/ANT-915-CW-HWR-SMA | Omnidirectional |
| **SMA Pigtail** | U.FL to SMA cable | 1 | $3.00 | Various/15cm length | Antenna connection |
| **Jumper Wires** | Dupont connector wires | 10 | $2.00 | Various/M-F 20cm | Prototyping connections |
| **Heat Shrink Tubing** | Assorted sizes | 1 pack | $3.00 | Various | Wire protection |
| **Foam Padding** | Anti-vibration foam | 1 sheet | $2.00 | Various | Component protection |

### Tools & Consumables

| Component | Description | Quantity | Estimated Cost | Supplier/Part Number | Notes |
|-----------|-------------|----------|----------------|---------------------|-------|
| **Prototype PCB** | Breadboard/perfboard | 1 | $2.00 | Various/5x7cm | Circuit assembly |
| **22AWG Wire** | Stranded hookup wire | 1 spool | $5.00 | Various/multiple colors | Connections |
| **Solder** | 60/40 rosin core solder | 1 tube | $5.00 | Kester/44 | Circuit assembly |
| **Flux** | No-clean flux paste | 1 tube | $3.00 | Various | Soldering aid |

## Total Estimated Cost

| Category | Subtotal |
|----------|----------|
| Core Components | $43.00 |
| Solar Power System | $12.25 |
| Sensors & Interface | $16.10 |
| Enclosure & Mounting | $20.00 |
| Connectivity & Accessories | $18.00 |
| Tools & Consumables | $15.00 |
| **TOTAL** | **$124.35** |

## Sourcing Notes

### Recommended Suppliers
- **Electronic Components**: Digi-Key, Mouser, SparkFun, Adafruit
- **Solar Panels**: Repurposed from solar path lights (cheaper option)
- **Enclosures**: 3D printing recommended for custom fit
- **Batteries**: Ensure authentic cells from reputable suppliers

### Cost Optimization Tips
1. **Solar Panel**: Harvest from $5 solar path lights
2. **Enclosure**: 3D print custom design (material cost ~$3)
3. **Bulk Ordering**: Order multiple units for better pricing
4. **Generic Components**: Use generic TP4056 modules and sensors

### Quality Considerations
- Use authentic ESP32 modules to avoid reliability issues
- Select high-quality 18650 batteries with protection circuits
- Ensure LoRa modules are properly certified for your region
- Use weatherproof connectors for outdoor deployment

### Regional Variations
- **North America**: 915MHz LoRa frequency
- **Europe**: 868MHz LoRa frequency  
- **Asia**: 433MHz LoRa frequency
- Verify local regulations for LoRa transmission power limits

## Assembly Time Estimate
- **Prototyping**: 4-6 hours
- **PCB Assembly**: 2-3 hours
- **Enclosure Preparation**: 1-2 hours
- **Testing & Calibration**: 2-3 hours
- **Total**: 9-14 hours per unit

## Maintenance Items
- Replace 18650 battery every 2-3 years
- Clean solar panel quarterly
- Check weatherproofing seals annually
- Update firmware as needed