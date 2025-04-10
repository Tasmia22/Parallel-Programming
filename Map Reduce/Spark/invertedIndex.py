from pyspark.sql import SparkSession
from pyspark import SparkContext
import sys
import os

def inverted_index_mapper(file_path):
    """
    Mapper function: Reads a file and emits (word, filename) pairs.
    """
    with open(file_path, 'r') as file:
        words = file.read().split()
        return [(word.lower(), os.path.basename(file_path)) for word in words]

def inverted_index_reducer(word_files):
    """
    Reducer function: Groups filenames by word.
    """
    word, files = word_files
    return word, list(set(files))  # Remove duplicates

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: spark-submit inverted_index.py <input_folder>")
        sys.exit(-1)
    
    # Initialize Spark
    spark = SparkSession.builder.appName("InvertedIndex").getOrCreate()
    sc = spark.sparkContext
    
    input_folder = sys.argv[1]
    files = [os.path.join(input_folder, f) for f in os.listdir(input_folder) if os.path.isfile(os.path.join(input_folder, f))]
    
    # Create an RDD from input files
    rdd = sc.parallelize(files)
    
    # Apply mapper function
    mapped_rdd = rdd.flatMap(inverted_index_mapper)
    
    # Group by key (word) and apply reducer function
    inverted_index_rdd = mapped_rdd.groupByKey().map(inverted_index_reducer)
    
    # Save output
    output_path = "output_inverted_index"
    inverted_index_rdd.saveAsTextFile(output_path)
    
    print(f"Inverted index saved to {output_path}")
    
    # Stop Spark
    spark.stop()
