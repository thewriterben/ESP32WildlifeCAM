"""
Image processing utilities for ESP32 Wildlife Camera backend.
Handles image manipulation, thumbnail generation, and optimization.
"""

import os
import logging
from typing import Tuple, Optional
from PIL import Image, ImageOps, ImageEnhance
from pathlib import Path
import hashlib

logger = logging.getLogger(__name__)

class ImageProcessor:
    """
    Image processing utilities for wildlife camera images.
    """
    
    def __init__(self, base_upload_path: str):
        self.base_upload_path = Path(base_upload_path)
        self.thumbnail_dir = self.base_upload_path / 'thumbnails'
        self.processed_dir = self.base_upload_path / 'processed'
        
        # Create directories if they don't exist
        self.thumbnail_dir.mkdir(parents=True, exist_ok=True)
        self.processed_dir.mkdir(parents=True, exist_ok=True)
        
        # Processing settings
        self.thumbnail_size = (300, 300)
        self.preview_size = (800, 600)
        self.max_image_size = (2048, 1536)
        self.jpeg_quality = 85
    
    def get_thumbnail(self, image_path: str, size: Tuple[int, int] = None) -> str:
        """
        Generate or retrieve thumbnail for an image.
        
        Args:
            image_path: Path to original image
            size: Thumbnail size tuple (width, height)
            
        Returns:
            Path to thumbnail file
        """
        try:
            size = size or self.thumbnail_size
            
            # Generate thumbnail filename
            original_path = Path(image_path)
            thumbnail_name = f"{original_path.stem}_thumb_{size[0]}x{size[1]}.jpg"
            thumbnail_path = self.thumbnail_dir / thumbnail_name
            
            # Check if thumbnail already exists
            if thumbnail_path.exists():
                return str(thumbnail_path)
            
            # Generate thumbnail
            with Image.open(image_path) as img:
                # Convert to RGB if necessary
                if img.mode != 'RGB':
                    img = img.convert('RGB')
                
                # Create thumbnail
                img.thumbnail(size, Image.Resampling.LANCZOS)
                
                # Save thumbnail
                img.save(thumbnail_path, 'JPEG', quality=self.jpeg_quality, optimize=True)
                
                logger.info(f"Generated thumbnail: {thumbnail_path}")
                return str(thumbnail_path)
                
        except Exception as e:
            logger.error(f"Failed to generate thumbnail for {image_path}: {str(e)}")
            raise
    
    def create_preview(self, image_path: str, size: Tuple[int, int] = None) -> str:
        """
        Create a preview-sized version of an image.
        
        Args:
            image_path: Path to original image
            size: Preview size tuple (width, height)
            
        Returns:
            Path to preview file
        """
        try:
            size = size or self.preview_size
            
            # Generate preview filename
            original_path = Path(image_path)
            preview_name = f"{original_path.stem}_preview_{size[0]}x{size[1]}.jpg"
            preview_path = self.processed_dir / preview_name
            
            # Check if preview already exists
            if preview_path.exists():
                return str(preview_path)
            
            # Generate preview
            with Image.open(image_path) as img:
                # Convert to RGB if necessary
                if img.mode != 'RGB':
                    img = img.convert('RGB')
                
                # Resize maintaining aspect ratio
                img.thumbnail(size, Image.Resampling.LANCZOS)
                
                # Save preview
                img.save(preview_path, 'JPEG', quality=self.jpeg_quality, optimize=True)
                
                logger.info(f"Generated preview: {preview_path}")
                return str(preview_path)
                
        except Exception as e:
            logger.error(f"Failed to generate preview for {image_path}: {str(e)}")
            raise
    
    def optimize_image(self, image_path: str, max_size: Tuple[int, int] = None) -> str:
        """
        Optimize image size and quality for storage.
        
        Args:
            image_path: Path to original image
            max_size: Maximum dimensions tuple (width, height)
            
        Returns:
            Path to optimized image
        """
        try:
            max_size = max_size or self.max_image_size
            
            # Generate optimized filename
            original_path = Path(image_path)
            optimized_name = f"{original_path.stem}_optimized.jpg"
            optimized_path = self.processed_dir / optimized_name
            
            # Check if optimized version already exists
            if optimized_path.exists():
                return str(optimized_path)
            
            # Optimize image
            with Image.open(image_path) as img:
                # Convert to RGB if necessary
                if img.mode != 'RGB':
                    img = img.convert('RGB')
                
                # Resize if too large
                if img.size[0] > max_size[0] or img.size[1] > max_size[1]:
                    img.thumbnail(max_size, Image.Resampling.LANCZOS)
                
                # Auto-orient based on EXIF
                img = ImageOps.exif_transpose(img)
                
                # Save optimized image
                img.save(optimized_path, 'JPEG', quality=self.jpeg_quality, optimize=True)
                
                logger.info(f"Optimized image: {optimized_path}")
                return str(optimized_path)
                
        except Exception as e:
            logger.error(f"Failed to optimize image {image_path}: {str(e)}")
            raise
    
    def enhance_image(self, image_path: str, enhance_params: dict = None) -> str:
        """
        Enhance image for better visibility and analysis.
        
        Args:
            image_path: Path to original image
            enhance_params: Enhancement parameters
            
        Returns:
            Path to enhanced image
        """
        try:
            # Default enhancement parameters
            default_params = {
                'brightness': 1.1,
                'contrast': 1.2,
                'sharpness': 1.1,
                'color': 1.0
            }
            
            enhance_params = enhance_params or default_params
            
            # Generate enhanced filename
            original_path = Path(image_path)
            enhanced_name = f"{original_path.stem}_enhanced.jpg"
            enhanced_path = self.processed_dir / enhanced_name
            
            # Check if enhanced version already exists
            if enhanced_path.exists():
                return str(enhanced_path)
            
            # Enhance image
            with Image.open(image_path) as img:
                # Convert to RGB if necessary
                if img.mode != 'RGB':
                    img = img.convert('RGB')
                
                # Apply enhancements
                if enhance_params.get('brightness', 1.0) != 1.0:
                    enhancer = ImageEnhance.Brightness(img)
                    img = enhancer.enhance(enhance_params['brightness'])
                
                if enhance_params.get('contrast', 1.0) != 1.0:
                    enhancer = ImageEnhance.Contrast(img)
                    img = enhancer.enhance(enhance_params['contrast'])
                
                if enhance_params.get('sharpness', 1.0) != 1.0:
                    enhancer = ImageEnhance.Sharpness(img)
                    img = enhancer.enhance(enhance_params['sharpness'])
                
                if enhance_params.get('color', 1.0) != 1.0:
                    enhancer = ImageEnhance.Color(img)
                    img = enhancer.enhance(enhance_params['color'])
                
                # Save enhanced image
                img.save(enhanced_path, 'JPEG', quality=90, optimize=True)
                
                logger.info(f"Enhanced image: {enhanced_path}")
                return str(enhanced_path)
                
        except Exception as e:
            logger.error(f"Failed to enhance image {image_path}: {str(e)}")
            raise
    
    def get_image_info(self, image_path: str) -> dict:
        """
        Get comprehensive information about an image.
        
        Args:
            image_path: Path to image file
            
        Returns:
            Dictionary with image information
        """
        try:
            with Image.open(image_path) as img:
                # Basic image info
                info = {
                    'filename': Path(image_path).name,
                    'format': img.format,
                    'mode': img.mode,
                    'size': img.size,
                    'width': img.size[0],
                    'height': img.size[1],
                    'file_size': os.path.getsize(image_path)
                }
                
                # EXIF data if available
                if hasattr(img, '_getexif') and img._getexif():
                    exif_data = img._getexif()
                    if exif_data:
                        info['exif'] = {}
                        # Extract common EXIF tags
                        exif_tags = {
                            271: 'make',
                            272: 'model', 
                            274: 'orientation',
                            306: 'datetime',
                            34665: 'exif_ifd'
                        }
                        
                        for tag_id, tag_name in exif_tags.items():
                            if tag_id in exif_data:
                                info['exif'][tag_name] = exif_data[tag_id]
                
                # Image hash for deduplication
                info['hash'] = self._calculate_image_hash(image_path)
                
                # Aspect ratio
                info['aspect_ratio'] = round(img.size[0] / img.size[1], 2)
                
                return info
                
        except Exception as e:
            logger.error(f"Failed to get image info for {image_path}: {str(e)}")
            return {}
    
    def _calculate_image_hash(self, image_path: str) -> str:
        """Calculate hash of image content for deduplication."""
        try:
            with open(image_path, 'rb') as f:
                file_hash = hashlib.md5()
                chunk = f.read(8192)
                while chunk:
                    file_hash.update(chunk)
                    chunk = f.read(8192)
                return file_hash.hexdigest()
        except Exception as e:
            logger.error(f"Failed to calculate hash for {image_path}: {str(e)}")
            return ""
    
    def batch_process(self, image_paths: list, operations: list = None) -> dict:
        """
        Process multiple images in batch.
        
        Args:
            image_paths: List of image file paths
            operations: List of operations to perform ('thumbnail', 'preview', 'optimize', 'enhance')
            
        Returns:
            Dictionary with processing results
        """
        operations = operations or ['thumbnail', 'preview']
        results = {}
        
        for image_path in image_paths:
            try:
                image_results = {'original': image_path}
                
                if 'thumbnail' in operations:
                    image_results['thumbnail'] = self.get_thumbnail(image_path)
                
                if 'preview' in operations:
                    image_results['preview'] = self.create_preview(image_path)
                
                if 'optimize' in operations:
                    image_results['optimized'] = self.optimize_image(image_path)
                
                if 'enhance' in operations:
                    image_results['enhanced'] = self.enhance_image(image_path)
                
                if 'info' in operations:
                    image_results['info'] = self.get_image_info(image_path)
                
                results[image_path] = image_results
                
            except Exception as e:
                logger.error(f"Batch processing failed for {image_path}: {str(e)}")
                results[image_path] = {'error': str(e)}
        
        return results
    
    def cleanup_processed_files(self, older_than_days: int = 30):
        """
        Clean up old processed files to save disk space.
        
        Args:
            older_than_days: Remove files older than this many days
        """
        try:
            import time
            cutoff_time = time.time() - (older_than_days * 24 * 60 * 60)
            
            removed_count = 0
            
            # Clean thumbnails
            for file_path in self.thumbnail_dir.glob('*'):
                if file_path.is_file() and file_path.stat().st_mtime < cutoff_time:
                    file_path.unlink()
                    removed_count += 1
            
            # Clean processed images
            for file_path in self.processed_dir.glob('*'):
                if file_path.is_file() and file_path.stat().st_mtime < cutoff_time:
                    file_path.unlink()
                    removed_count += 1
            
            logger.info(f"Cleaned up {removed_count} old processed files")
            
        except Exception as e:
            logger.error(f"Failed to cleanup processed files: {str(e)}")
    
    def get_storage_stats(self) -> dict:
        """Get storage statistics for processed images."""
        try:
            stats = {
                'thumbnail_count': 0,
                'thumbnail_size': 0,
                'processed_count': 0,
                'processed_size': 0
            }
            
            # Count thumbnails
            for file_path in self.thumbnail_dir.glob('*'):
                if file_path.is_file():
                    stats['thumbnail_count'] += 1
                    stats['thumbnail_size'] += file_path.stat().st_size
            
            # Count processed images
            for file_path in self.processed_dir.glob('*'):
                if file_path.is_file():
                    stats['processed_count'] += 1
                    stats['processed_size'] += file_path.stat().st_size
            
            # Convert to human readable
            stats['thumbnail_size_mb'] = round(stats['thumbnail_size'] / (1024 * 1024), 2)
            stats['processed_size_mb'] = round(stats['processed_size'] / (1024 * 1024), 2)
            stats['total_size_mb'] = stats['thumbnail_size_mb'] + stats['processed_size_mb']
            
            return stats
            
        except Exception as e:
            logger.error(f"Failed to get storage stats: {str(e)}")
            return {}