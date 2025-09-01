#!/usr/bin/env python3
"""
Virtual Wildlife Monitoring Deployment Simulator
Phase 2 Enhancement - Interactive Educational Tool

This tool provides a comprehensive simulation environment for planning
and testing wildlife monitoring deployments without physical hardware.

Features:
- Realistic wildlife camera placement simulation
- Environmental condition modeling
- Species behavior simulation
- Data collection and analysis training
- Cost estimation and deployment planning

Usage:
python virtual_deployment.py

Requirements:
pip install tkinter matplotlib numpy
"""

import tkinter as tk
from tkinter import ttk, messagebox
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import random
from datetime import datetime, timedelta

class VirtualDeploymentSimulator:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Wildlife Monitoring - Virtual Deployment Simulator")
        self.root.geometry("1000x700")
        
        # Simulation parameters
        self.deployments = []
        self.species_list = [
            {"name": "White-tailed Deer", "activity": "crepuscular", "detection_prob": 0.7},
            {"name": "Red Fox", "activity": "nocturnal", "detection_prob": 0.5},
            {"name": "Black Bear", "activity": "diurnal", "detection_prob": 0.8},
            {"name": "Gray Squirrel", "activity": "diurnal", "detection_prob": 0.6},
            {"name": "Great Horned Owl", "activity": "nocturnal", "detection_prob": 0.4}
        ]
        
        self.setup_ui()

    def setup_ui(self):
        # Main frame
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Left panel for controls
        control_frame = ttk.LabelFrame(main_frame, text="Deployment Configuration")
        control_frame.pack(side=tk.LEFT, fill=tk.Y, padx=5)
        
        # Deployment inputs
        ttk.Label(control_frame, text="Location X (km):").pack(anchor=tk.W)
        self.x_var = tk.DoubleVar(value=0.0)
        ttk.Entry(control_frame, textvariable=self.x_var, width=15).pack(anchor=tk.W, pady=2)
        
        ttk.Label(control_frame, text="Location Y (km):").pack(anchor=tk.W)
        self.y_var = tk.DoubleVar(value=0.0)
        ttk.Entry(control_frame, textvariable=self.y_var, width=15).pack(anchor=tk.W, pady=2)
        
        ttk.Label(control_frame, text="Habitat Type:").pack(anchor=tk.W)
        self.habitat_var = tk.StringVar(value="Forest")
        habitat_combo = ttk.Combobox(control_frame, textvariable=self.habitat_var,
                                   values=["Forest", "Grassland", "Wetland", "Mixed"], width=12)
        habitat_combo.pack(anchor=tk.W, pady=2)
        
        ttk.Label(control_frame, text="Camera Model:").pack(anchor=tk.W)
        self.camera_var = tk.StringVar(value="ESP32-S3-CAM")
        camera_combo = ttk.Combobox(control_frame, textvariable=self.camera_var,
                                  values=["ESP32-CAM Basic", "ESP32-S3-CAM", "Professional"], width=12)
        camera_combo.pack(anchor=tk.W, pady=2)
        
        # Solar panel option
        self.solar_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(control_frame, text="Solar Panel", variable=self.solar_var).pack(anchor=tk.W, pady=5)
        
        # Buttons
        ttk.Button(control_frame, text="Add Camera", command=self.add_deployment).pack(pady=5)
        ttk.Button(control_frame, text="Run Simulation", command=self.run_simulation).pack(pady=5)
        ttk.Button(control_frame, text="Clear All", command=self.clear_all).pack(pady=5)
        
        # Simulation parameters
        ttk.Label(control_frame, text="Simulation Days:").pack(anchor=tk.W, pady=(10,0))
        self.days_var = tk.IntVar(value=30)
        ttk.Entry(control_frame, textvariable=self.days_var, width=15).pack(anchor=tk.W, pady=2)
        
        # Results area
        results_frame = ttk.LabelFrame(main_frame, text="Results")
        results_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=5)
        
        # Results text
        self.results_text = tk.Text(results_frame, wrap=tk.WORD, height=15)
        scrollbar = ttk.Scrollbar(results_frame, orient=tk.VERTICAL, command=self.results_text.yview)
        self.results_text.configure(yscrollcommand=scrollbar.set)
        self.results_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Visualization area
        viz_frame = ttk.LabelFrame(main_frame, text="Deployment Map")
        viz_frame.pack(side=tk.BOTTOM, fill=tk.X, pady=5)
        
        self.fig, self.ax = plt.subplots(figsize=(10, 4))
        self.canvas = FigureCanvasTkAgg(self.fig, viz_frame)
        self.canvas.get_tk_widget().pack()
        
        self.update_map()

    def add_deployment(self):
        deployment = {
            'x': self.x_var.get(),
            'y': self.y_var.get(),
            'habitat': self.habitat_var.get(),
            'camera': self.camera_var.get(),
            'solar': self.solar_var.get(),
            'cost': self.calculate_cost()
        }
        
        self.deployments.append(deployment)
        self.update_map()
        
        self.results_text.insert(tk.END, f"Added camera at ({deployment['x']:.1f}, {deployment['y']:.1f})\\n")
        self.results_text.insert(tk.END, f"  Habitat: {deployment['habitat']}\\n")
        self.results_text.insert(tk.END, f"  Model: {deployment['camera']}\\n")
        self.results_text.insert(tk.END, f"  Cost: ${deployment['cost']:.2f}\\n\\n")
        self.results_text.see(tk.END)

    def calculate_cost(self):
        base_costs = {
            "ESP32-CAM Basic": 45,
            "ESP32-S3-CAM": 65,
            "Professional": 285
        }
        
        cost = base_costs[self.camera_var.get()]
        if self.solar_var.get():
            cost += 45  # Solar panel
        cost += 35  # Housing
        cost += 25  # Installation
        
        return cost

    def run_simulation(self):
        if not self.deployments:
            messagebox.showwarning("No Deployments", "Please add at least one camera first.")
            return
        
        self.results_text.delete(1.0, tk.END)
        self.results_text.insert(tk.END, "Running simulation...\\n\\n")
        
        days = self.days_var.get()
        total_detections = 0
        total_cost = sum(d['cost'] for d in self.deployments)
        
        for i, deployment in enumerate(self.deployments):
            self.results_text.insert(tk.END, f"Camera {i+1} - {deployment['camera']}\\n")
            self.results_text.insert(tk.END, f"Location: ({deployment['x']:.1f}, {deployment['y']:.1f})\\n")
            self.results_text.insert(tk.END, f"Habitat: {deployment['habitat']}\\n")
            
            # Simulate detections
            detections = self.simulate_detections(deployment, days)
            total_detections += len(detections)
            
            # Count species
            species_count = {}
            for detection in detections:
                species = detection['species']
                species_count[species] = species_count.get(species, 0) + 1
            
            self.results_text.insert(tk.END, f"Total detections: {len(detections)}\\n")
            for species, count in species_count.items():
                self.results_text.insert(tk.END, f"  {species}: {count}\\n")
            self.results_text.insert(tk.END, "\\n")
        
        # Summary
        self.results_text.insert(tk.END, "=== SUMMARY ===\\n")
        self.results_text.insert(tk.END, f"Total cameras: {len(self.deployments)}\\n")
        self.results_text.insert(tk.END, f"Total cost: ${total_cost:.2f}\\n")
        self.results_text.insert(tk.END, f"Cost per camera: ${total_cost/len(self.deployments):.2f}\\n")
        self.results_text.insert(tk.END, f"Total detections: {total_detections}\\n")
        self.results_text.insert(tk.END, f"Detections per camera: {total_detections/len(self.deployments):.1f}\\n")
        
        self.results_text.see(tk.END)

    def simulate_detections(self, deployment, days):
        detections = []
        habitat_bonus = {
            "Forest": {"White-tailed Deer": 1.2, "Black Bear": 1.5, "Gray Squirrel": 1.3},
            "Grassland": {"White-tailed Deer": 1.5, "Red Fox": 1.2},
            "Wetland": {"Red Fox": 1.1, "Great Horned Owl": 1.3},
            "Mixed": {}  # No specific bonuses
        }
        
        for day in range(days):
            for species_info in self.species_list:
                species = species_info["name"]
                base_prob = species_info["detection_prob"]
                
                # Apply habitat bonus
                bonus = habitat_bonus.get(deployment["habitat"], {}).get(species, 1.0)
                detection_prob = base_prob * bonus
                
                # Random chance of detection
                if random.random() < detection_prob * 0.1:  # Scale down for realistic numbers
                    detection_time = datetime.now() + timedelta(days=day, hours=random.randint(0, 23))
                    detections.append({
                        "species": species,
                        "timestamp": detection_time,
                        "confidence": random.uniform(0.6, 0.95)
                    })
        
        return detections

    def update_map(self):
        self.ax.clear()
        
        if self.deployments:
            x_coords = [d['x'] for d in self.deployments]
            y_coords = [d['y'] for d in self.deployments]
            
            colors = {'Forest': 'green', 'Grassland': 'yellow', 'Wetland': 'blue', 'Mixed': 'purple'}
            for deployment in self.deployments:
                color = colors.get(deployment['habitat'], 'gray')
                self.ax.scatter(deployment['x'], deployment['y'], c=color, s=100, alpha=0.7)
                self.ax.annotate(deployment['camera'][:8], 
                               (deployment['x'], deployment['y']),
                               xytext=(5, 5), textcoords='offset points', fontsize=8)
            
            self.ax.set_xlabel('X Coordinate (km)')
            self.ax.set_ylabel('Y Coordinate (km)')
            self.ax.set_title(f'Wildlife Camera Deployments ({len(self.deployments)} cameras)')
            self.ax.grid(True)
        else:
            self.ax.set_title('No deployments yet')
            self.ax.set_xlabel('X Coordinate (km)')
            self.ax.set_ylabel('Y Coordinate (km)')
            self.ax.grid(True)
        
        self.canvas.draw()

    def clear_all(self):
        self.deployments.clear()
        self.results_text.delete(1.0, tk.END)
        self.update_map()

if __name__ == "__main__":
    root = tk.Tk()
    app = VirtualDeploymentSimulator(root)
    root.mainloop()