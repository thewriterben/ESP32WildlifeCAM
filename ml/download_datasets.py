#!/usr/bin/env python3
"""
Wildlife Dataset Download and Management Script

This script automates the download and organization of wildlife datasets
from various sources including iNaturalist, Wildlife Insights, and custom
repositories for the ESP32 Wildlife Camera ML pipeline.
"""

import os
import json
import logging
import requests
import zipfile
import tarfile
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import hashlib
from urllib.parse import urlparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import time

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeDatasetDownloader:
    """
    Automated wildlife dataset download and management.
    
    Features:
    - Multiple data source support (iNaturalist, Wildlife Insights, Kaggle)
    - Automated dataset organization
    - Integrity verification
    - Resume interrupted downloads
    - Species filtering and selection
    """
    
    def __init__(self, config: Optional[Dict] = None):
        self.config = self._load_config(config)
        self.download_dir = Path(self.config['download_dir'])
        self.datasets_dir = Path(self.config['datasets_dir'])
        
        # Create directories
        self.download_dir.mkdir(parents=True, exist_ok=True)
        self.datasets_dir.mkdir(parents=True, exist_ok=True)
        
        # Dataset registry
        self.dataset_registry = self._load_dataset_registry()
        
        logger.info(f"Dataset downloader initialized. Download dir: {self.download_dir}")
    
    def _load_config(self, config: Optional[Dict]) -> Dict:
        """Load downloader configuration."""
        default_config = {
            'download_dir': './downloads',
            'datasets_dir': './datasets',
            'max_concurrent_downloads': 3,
            'chunk_size': 8192,
            'verify_integrity': True,
            'target_species': [
                'deer', 'bear', 'fox', 'wolf', 'raccoon', 'rabbit',
                'bird', 'squirrel', 'coyote', 'bobcat', 'elk', 'moose'
            ],
            'min_images_per_species': 100,
            'max_images_per_species': 2000,
            'image_quality_threshold': 0.7
        }
        
        if config:
            default_config.update(config)
        
        return default_config
    
    def _load_dataset_registry(self) -> Dict:
        """Load registry of available datasets."""
        return {
            'wildlife_insights_sample': {
                'name': 'Wildlife Insights Sample Dataset',
                'url': 'https://example.com/wi_sample.zip',  # Placeholder URL
                'size_mb': 150,
                'species_count': 15,
                'image_count': 1500,
                'format': 'directory',
                'checksum': 'sha256:placeholder_checksum',
                'description': 'Sample dataset from Wildlife Insights with 15 common species'
            },
            'inaturalist_mammals': {
                'name': 'iNaturalist Mammals Dataset',
                'url': 'https://example.com/inat_mammals.tar.gz',  # Placeholder URL
                'size_mb': 500,
                'species_count': 25,
                'image_count': 5000,
                'format': 'annotations',
                'checksum': 'sha256:placeholder_checksum',
                'description': 'Curated mammal dataset from iNaturalist'
            },
            'camera_trap_animals': {
                'name': 'Camera Trap Animals Dataset',
                'url': 'https://example.com/camera_trap.zip',  # Placeholder URL
                'size_mb': 800,
                'species_count': 20,
                'image_count': 8000,
                'format': 'directory',
                'checksum': 'sha256:placeholder_checksum',
                'description': 'Real-world camera trap images from various locations'
            },
            'synthetic_wildlife': {
                'name': 'Synthetic Wildlife Dataset',
                'url': 'https://example.com/synthetic.zip',  # Placeholder URL
                'size_mb': 200,
                'species_count': 12,
                'image_count': 2400,
                'format': 'directory',
                'checksum': 'sha256:placeholder_checksum',
                'description': 'AI-generated wildlife images for data augmentation'
            }
        }
    
    def list_available_datasets(self) -> Dict:
        """List all available datasets."""
        logger.info("Available datasets:")
        for dataset_id, info in self.dataset_registry.items():
            print(f"\n{dataset_id}:")
            print(f"  Name: {info['name']}")
            print(f"  Size: {info['size_mb']} MB")
            print(f"  Species: {info['species_count']}")
            print(f"  Images: {info['image_count']}")
            print(f"  Description: {info['description']}")
        
        return self.dataset_registry
    
    def download_dataset(self, dataset_id: str, force_redownload: bool = False) -> bool:
        """
        Download a specific dataset.
        
        Args:
            dataset_id: ID of dataset to download
            force_redownload: Force redownload even if exists
            
        Returns:
            True if successful, False otherwise
        """
        if dataset_id not in self.dataset_registry:
            logger.error(f"Dataset '{dataset_id}' not found in registry")
            return False
        
        dataset_info = self.dataset_registry[dataset_id]
        logger.info(f"Downloading dataset: {dataset_info['name']}")
        
        # Check if already downloaded
        download_path = self.download_dir / f"{dataset_id}.{self._get_file_extension(dataset_info['url'])}"
        
        if download_path.exists() and not force_redownload:
            logger.info(f"Dataset already downloaded: {download_path}")
            if self.config['verify_integrity']:
                if self._verify_file_integrity(download_path, dataset_info.get('checksum')):
                    return True
                else:
                    logger.warning("Integrity check failed, re-downloading...")
        
        # Download the dataset
        try:
            success = self._download_file(dataset_info['url'], download_path)
            if success and self.config['verify_integrity']:
                if not self._verify_file_integrity(download_path, dataset_info.get('checksum')):
                    logger.error("Downloaded file failed integrity check")
                    return False
            
            return success
            
        except Exception as e:
            logger.error(f"Failed to download dataset {dataset_id}: {e}")
            return False
    
    def _download_file(self, url: str, destination: Path) -> bool:
        """Download a file with progress tracking and resume capability."""
        try:
            # Check if partial download exists
            resume_pos = 0
            if destination.exists():
                resume_pos = destination.stat().st_size
                logger.info(f"Resuming download from position {resume_pos}")
            
            # Setup headers for resume
            headers = {}
            if resume_pos > 0:
                headers['Range'] = f'bytes={resume_pos}-'
            
            # Start download
            response = requests.get(url, headers=headers, stream=True, timeout=30)
            response.raise_for_status()
            
            # Get total size
            total_size = int(response.headers.get('content-length', 0))
            if resume_pos > 0:
                total_size += resume_pos
            
            # Download with progress
            mode = 'ab' if resume_pos > 0 else 'wb'
            downloaded = resume_pos
            
            with open(destination, mode) as f:
                start_time = time.time()
                for chunk in response.iter_content(chunk_size=self.config['chunk_size']):
                    if chunk:
                        f.write(chunk)
                        downloaded += len(chunk)
                        
                        # Progress update every 5MB
                        if downloaded % (5 * 1024 * 1024) < len(chunk):
                            elapsed = time.time() - start_time
                            speed = downloaded / elapsed / (1024 * 1024)  # MB/s
                            if total_size > 0:
                                progress = (downloaded / total_size) * 100
                                logger.info(f"Download progress: {progress:.1f}% ({speed:.1f} MB/s)")
                            else:
                                logger.info(f"Downloaded: {downloaded/(1024*1024):.1f} MB ({speed:.1f} MB/s)")
            
            logger.info(f"Download completed: {destination}")
            return True
            
        except requests.exceptions.RequestException as e:
            logger.error(f"Download failed: {e}")
            return False
        except Exception as e:
            logger.error(f"Unexpected error during download: {e}")
            return False
    
    def _get_file_extension(self, url: str) -> str:
        """Get file extension from URL."""
        parsed = urlparse(url)
        path = Path(parsed.path)
        return path.suffix.lstrip('.') or 'zip'
    
    def _verify_file_integrity(self, file_path: Path, expected_checksum: Optional[str]) -> bool:
        """Verify file integrity using checksum."""
        if not expected_checksum:
            logger.info("No checksum provided, skipping integrity check")
            return True
        
        if not expected_checksum.startswith('sha256:'):
            logger.warning("Unsupported checksum format, skipping verification")
            return True
        
        expected_hash = expected_checksum.replace('sha256:', '')
        
        try:
            sha256_hash = hashlib.sha256()
            with open(file_path, 'rb') as f:
                for chunk in iter(lambda: f.read(4096), b""):
                    sha256_hash.update(chunk)
            
            actual_hash = sha256_hash.hexdigest()
            
            if actual_hash == expected_hash:
                logger.info("File integrity verified successfully")
                return True
            else:
                logger.error(f"Integrity check failed. Expected: {expected_hash}, Got: {actual_hash}")
                return False
                
        except Exception as e:
            logger.error(f"Error during integrity check: {e}")
            return False
    
    def extract_dataset(self, dataset_id: str, organize: bool = True) -> bool:
        """
        Extract downloaded dataset.
        
        Args:
            dataset_id: ID of dataset to extract
            organize: Whether to organize extracted files
            
        Returns:
            True if successful, False otherwise
        """
        if dataset_id not in self.dataset_registry:
            logger.error(f"Dataset '{dataset_id}' not found in registry")
            return False
        
        dataset_info = self.dataset_registry[dataset_id]
        
        # Find downloaded file
        download_path = None
        for ext in ['zip', 'tar.gz', 'tar', 'gz']:
            candidate = self.download_dir / f"{dataset_id}.{ext}"
            if candidate.exists():
                download_path = candidate
                break
        
        if not download_path:
            logger.error(f"Downloaded file not found for dataset '{dataset_id}'")
            return False
        
        # Extract
        extract_dir = self.datasets_dir / dataset_id
        logger.info(f"Extracting {download_path} to {extract_dir}")
        
        try:
            extract_dir.mkdir(parents=True, exist_ok=True)
            
            if download_path.suffix.lower() == '.zip':
                with zipfile.ZipFile(download_path, 'r') as zip_ref:
                    zip_ref.extractall(extract_dir)
            elif download_path.suffix.lower() in ['.tar', '.gz'] or '.tar.gz' in download_path.name:
                with tarfile.open(download_path, 'r:*') as tar_ref:
                    tar_ref.extractall(extract_dir)
            else:
                logger.error(f"Unsupported archive format: {download_path.suffix}")
                return False
            
            logger.info(f"Extraction completed: {extract_dir}")
            
            # Organize if requested
            if organize:
                self._organize_extracted_dataset(extract_dir, dataset_info)
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to extract dataset {dataset_id}: {e}")
            return False
    
    def _organize_extracted_dataset(self, extract_dir: Path, dataset_info: Dict):
        """Organize extracted dataset into standard structure."""
        logger.info(f"Organizing dataset in {extract_dir}")
        
        # Create standard structure
        organized_dir = extract_dir / 'organized'
        train_dir = organized_dir / 'train'
        val_dir = organized_dir / 'validation'
        test_dir = organized_dir / 'test'
        
        for split_dir in [train_dir, val_dir, test_dir]:
            split_dir.mkdir(parents=True, exist_ok=True)
        
        # Find all image files
        image_extensions = {'.jpg', '.jpeg', '.png', '.bmp', '.tiff'}
        image_files = []
        
        for file_path in extract_dir.rglob('*'):
            if file_path.suffix.lower() in image_extensions:
                image_files.append(file_path)
        
        logger.info(f"Found {len(image_files)} image files")
        
        # Group by species (attempt to extract from path/filename)
        species_groups = {}
        target_species = set(self.config['target_species'])
        
        for image_file in image_files:
            species = self._extract_species_from_path(image_file, target_species)
            if species:
                if species not in species_groups:
                    species_groups[species] = []
                species_groups[species].append(image_file)
        
        # Create species directories and copy files
        organization_summary = {'species': {}, 'total_organized': 0}
        
        for species, files in species_groups.items():
            if len(files) < self.config['min_images_per_species']:
                logger.warning(f"Skipping {species}: only {len(files)} images (minimum: {self.config['min_images_per_species']})")
                continue
            
            # Limit files if too many
            if len(files) > self.config['max_images_per_species']:
                files = files[:self.config['max_images_per_species']]
                logger.info(f"Limited {species} to {len(files)} images")
            
            # Create species directories
            for split_dir in [train_dir, val_dir, test_dir]:
                (split_dir / species).mkdir(exist_ok=True)
            
            # Split files (70% train, 20% val, 10% test)
            import random
            random.shuffle(files)
            
            train_count = int(len(files) * 0.7)
            val_count = int(len(files) * 0.2)
            
            train_files = files[:train_count]
            val_files = files[train_count:train_count + val_count]
            test_files = files[train_count + val_count:]
            
            # Copy files
            self._copy_files(train_files, train_dir / species)
            self._copy_files(val_files, val_dir / species)
            self._copy_files(test_files, test_dir / species)
            
            organization_summary['species'][species] = {
                'train': len(train_files),
                'validation': len(val_files),
                'test': len(test_files),
                'total': len(files)
            }
            organization_summary['total_organized'] += len(files)
        
        # Save organization summary
        with open(organized_dir / 'organization_summary.json', 'w') as f:
            json.dump(organization_summary, f, indent=2)
        
        logger.info(f"Dataset organization completed. Summary: {organization_summary}")
    
    def _extract_species_from_path(self, file_path: Path, target_species: set) -> Optional[str]:
        """Extract species name from file path."""
        path_str = str(file_path).lower()
        
        for species in target_species:
            if species.lower() in path_str:
                return species
        
        return None
    
    def _copy_files(self, files: List[Path], destination_dir: Path):
        """Copy files to destination directory."""
        import shutil
        
        for file_path in files:
            try:
                destination_path = destination_dir / file_path.name
                shutil.copy2(str(file_path), str(destination_path))
            except Exception as e:
                logger.error(f"Failed to copy {file_path}: {e}")
    
    def download_multiple_datasets(self, dataset_ids: List[str], 
                                 extract: bool = True,
                                 organize: bool = True) -> Dict[str, bool]:
        """
        Download multiple datasets concurrently.
        
        Args:
            dataset_ids: List of dataset IDs to download
            extract: Whether to extract after download
            organize: Whether to organize extracted files
            
        Returns:
            Dictionary mapping dataset IDs to success status
        """
        logger.info(f"Downloading {len(dataset_ids)} datasets...")
        
        results = {}
        
        # Download datasets
        with ThreadPoolExecutor(max_workers=self.config['max_concurrent_downloads']) as executor:
            # Submit download tasks
            future_to_dataset = {
                executor.submit(self.download_dataset, dataset_id): dataset_id
                for dataset_id in dataset_ids
            }
            
            # Collect results
            for future in as_completed(future_to_dataset):
                dataset_id = future_to_dataset[future]
                try:
                    success = future.result()
                    results[dataset_id] = success
                    
                    if success and extract:
                        # Extract in main thread to avoid conflicts
                        extract_success = self.extract_dataset(dataset_id, organize)
                        results[dataset_id] = results[dataset_id] and extract_success
                        
                except Exception as e:
                    logger.error(f"Error processing dataset {dataset_id}: {e}")
                    results[dataset_id] = False
        
        # Summary
        successful = sum(1 for success in results.values() if success)
        logger.info(f"Download completed: {successful}/{len(dataset_ids)} successful")
        
        return results
    
    def create_combined_dataset(self, dataset_ids: List[str], 
                              output_name: str = 'combined_wildlife') -> bool:
        """
        Combine multiple downloaded datasets into a single organized dataset.
        
        Args:
            dataset_ids: List of dataset IDs to combine
            output_name: Name for combined dataset
            
        Returns:
            True if successful, False otherwise
        """
        logger.info(f"Creating combined dataset '{output_name}' from {len(dataset_ids)} datasets")
        
        combined_dir = self.datasets_dir / output_name
        train_dir = combined_dir / 'train'
        val_dir = combined_dir / 'validation'
        test_dir = combined_dir / 'test'
        
        # Create directories
        for split_dir in [train_dir, val_dir, test_dir]:
            split_dir.mkdir(parents=True, exist_ok=True)
        
        combination_summary = {'datasets_combined': dataset_ids, 'species_counts': {}}
        
        # Process each dataset
        for dataset_id in dataset_ids:
            dataset_dir = self.datasets_dir / dataset_id / 'organized'
            
            if not dataset_dir.exists():
                logger.warning(f"Organized dataset not found: {dataset_dir}")
                continue
            
            # Copy from each split
            for split_name in ['train', 'validation', 'test']:
                source_split_dir = dataset_dir / split_name
                dest_split_dir = combined_dir / split_name
                
                if not source_split_dir.exists():
                    continue
                
                # Process each species
                for species_dir in source_split_dir.iterdir():
                    if not species_dir.is_dir():
                        continue
                    
                    species_name = species_dir.name
                    dest_species_dir = dest_split_dir / species_name
                    dest_species_dir.mkdir(exist_ok=True)
                    
                    # Copy images
                    image_count = 0
                    for image_file in species_dir.iterdir():
                        if image_file.suffix.lower() in {'.jpg', '.jpeg', '.png', '.bmp'}:
                            dest_file = dest_species_dir / f"{dataset_id}_{image_file.name}"
                            try:
                                import shutil
                                shutil.copy2(str(image_file), str(dest_file))
                                image_count += 1
                            except Exception as e:
                                logger.error(f"Failed to copy {image_file}: {e}")
                    
                    # Update summary
                    species_key = f"{species_name}_{split_name}"
                    combination_summary['species_counts'][species_key] = \
                        combination_summary['species_counts'].get(species_key, 0) + image_count
        
        # Save combination summary
        with open(combined_dir / 'combination_summary.json', 'w') as f:
            json.dump(combination_summary, f, indent=2)
        
        logger.info(f"Combined dataset created: {combined_dir}")
        return True
    
    def get_dataset_info(self, dataset_path: str) -> Dict:
        """Get information about a local dataset."""
        dataset_dir = Path(dataset_path)
        
        if not dataset_dir.exists():
            return {'error': 'Dataset directory not found'}
        
        info = {
            'path': str(dataset_dir),
            'splits': {},
            'total_images': 0,
            'species_count': 0,
            'species_list': []
        }
        
        # Analyze each split
        for split_name in ['train', 'validation', 'test']:
            split_dir = dataset_dir / split_name
            if split_dir.exists():
                split_info = self._analyze_split_directory(split_dir)
                info['splits'][split_name] = split_info
                info['total_images'] += split_info['total_images']
        
        # Get overall species information
        all_species = set()
        for split_info in info['splits'].values():
            all_species.update(split_info['species'])
        
        info['species_count'] = len(all_species)
        info['species_list'] = sorted(list(all_species))
        
        return info
    
    def _analyze_split_directory(self, split_dir: Path) -> Dict:
        """Analyze a split directory."""
        split_info = {
            'total_images': 0,
            'species': [],
            'species_counts': {}
        }
        
        for species_dir in split_dir.iterdir():
            if not species_dir.is_dir():
                continue
            
            species_name = species_dir.name
            split_info['species'].append(species_name)
            
            # Count images
            image_count = 0
            for file_path in species_dir.iterdir():
                if file_path.suffix.lower() in {'.jpg', '.jpeg', '.png', '.bmp'}:
                    image_count += 1
            
            split_info['species_counts'][species_name] = image_count
            split_info['total_images'] += image_count
        
        return split_info


def main():
    """Main function for command-line usage."""
    import argparse
    
    parser = argparse.ArgumentParser(description='Download and manage wildlife datasets')
    parser.add_argument('--list', action='store_true', help='List available datasets')
    parser.add_argument('--download', nargs='+', help='Download specific datasets')
    parser.add_argument('--download-all', action='store_true', help='Download all available datasets')
    parser.add_argument('--extract', action='store_true', help='Extract downloaded datasets')
    parser.add_argument('--organize', action='store_true', help='Organize extracted datasets')
    parser.add_argument('--combine', nargs='+', help='Combine multiple datasets')
    parser.add_argument('--info', type=str, help='Get info about a dataset directory')
    parser.add_argument('--config', type=str, help='Configuration file path')
    
    args = parser.parse_args()
    
    # Load configuration
    config = {}
    if args.config and os.path.exists(args.config):
        with open(args.config, 'r') as f:
            import yaml
            config = yaml.safe_load(f).get('dataset_download', {})
    
    # Initialize downloader
    downloader = WildlifeDatasetDownloader(config)
    
    try:
        if args.list:
            downloader.list_available_datasets()
        
        elif args.download:
            results = downloader.download_multiple_datasets(
                args.download, 
                extract=args.extract,
                organize=args.organize
            )
            print(f"\nDownload results: {results}")
        
        elif args.download_all:
            all_datasets = list(downloader.dataset_registry.keys())
            results = downloader.download_multiple_datasets(
                all_datasets,
                extract=args.extract,
                organize=args.organize
            )
            print(f"\nDownload results: {results}")
        
        elif args.combine:
            success = downloader.create_combined_dataset(args.combine)
            print(f"Combined dataset creation: {'Success' if success else 'Failed'}")
        
        elif args.info:
            info = downloader.get_dataset_info(args.info)
            print(f"\nDataset Info:\n{json.dumps(info, indent=2)}")
        
        else:
            print("No action specified. Use --help for available options.")
            
    except Exception as e:
        logger.error(f"Operation failed: {str(e)}")
        raise


if __name__ == "__main__":
    main()