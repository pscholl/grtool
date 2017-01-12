% grt-train-skl
%
%

# NAME

 grt-train-skl

# SYNOPSIS
grt train-skl

grt train-skl ESTIMATOR [-h] [-g] [-p] [-f FILE] [-n TRAINSET]  
                        [--custom-args dict] [--custom-args-compat]  
                        [ESTIMATOR SPECIFIC OPTIONS]  
                        [SAMPLES]

unsupervised clustering algorithms (estimators) from scikit-learn  
[train module]

Default output: estimator dump

# DESCRIPTION
The **_grtool train-skl_** module is a python wrapper around these algorithms, which are provided by the *sklearn.cluster* class, and respectively the *sklearn.mixture* class in case of the gaussian mixture implementations. Like the **_train_** module, it processes a stream of samples, e.g. in feature space, however since a majority of the provided algorithms are direct clustering methods, it also provides an option to output the learned cluster labeling, i.e. the predictions, in addition to saving the created classifier in a file. Only in the case of using gaussian mixture models an actual model is created, which can then be used with the **_predict-skl_** module to classify new data similar to the **_predict_** module.

# OPTIONS
positional arguments:
-  SAMPLES               sample stream, format: [label] [[features]] (default: -)

optional arguments:
-  -h, --help            show this help message and exit
-  -g, --graph           graph results of estimator fitting (default: False)
-  -p, --prediction      output the prediction for the test set, or if no test set specified, the train set (default: False)
-  -f FILE, --file FILE  save the estimator model to the file (default: None)
-  -n TRAINSET, --trainset TRAINSET training/test set customization:
  - <= 0: no split
  - < 1: random stratified split, rest is test set
  - \> 1: k-fold split, selected fold is test set (k.x, [folds].[select])
  - file: use this file as sample source, no split  
(default: 0)
-  --custom-args dict    custom trainer arguments (default: None)
-  --custom-args-compat  compatibility mode for --custom-args, e.g. when used with GNU parallell  
keys and values are strings only! format: {key1:value1:key2...} (default: False)

At this point the only available custom argument is [{"connectivity":"temp_seq"}]. It enables *temporal sequence connectivity* for agglomerative clustering, which restricts the clustering by imposing a connectivity on each sample to its direct neighbors in the sample stream.


# ESTIMATOR SPECIFIC OPTIONS
Once an algorithm has been chosen, more command line parameters become available.

ESTIMATOR:  
used to fit model to sample features  
See http://scikit-learn.org/stable/modules/clustering.html for more info.

- AffinityPropagation: Perform Affinity Propagation Clustering of data.  
- AgglomerativeClustering: Agglomerative Clustering  
- BayesianGaussianMixture: Variational Bayesian estimation of a Gaussian mixture.  
- Birch: Implements the Birch clustering algorithm.  
- DBSCAN: Perform DBSCAN clustering from vector array or distance matrix.  
- FeatureAgglomeration: Agglomerate features.  
- GaussianMixture: Gaussian Mixture Model  
- KMeans: K-Means clustering  
- MeanShift: Mean shift clustering using a flat kernel.  
- MiniBatchKMeans: Mini-Batch K-Means clustering  
- SpectralClusterin: Apply clustering to a projection to the normalized laplacian.

## KMeans
Its primary parameter is the number of clusters sought after and the metric used to compare samples is their distance. Since the algorithm can converge to a local minimum, the centroid initialization is an important feature of *scikit-learn*'s implementation. Rather than initializing randomly, the option to use the *kmeans++* initialization method is given. It chooses initial centroids from a random distribution dependent on data point distances and thus distributes centroids generally distant but still randomized. According to *scikit-learn*'s documentation, it is a general-purpose method for a large number of samples and small number of clusters. A more computationally efficient Mini Batch variant is also implemented.

estimator parameters:
-  --algorithm str         (default: auto)
-  --copy_x bool           (default: True)
-  --init str              (default: k-means++)
-  --max_iter int          (default: 300)
-  --n_clusters int        (default: 8)
-  --n_init int            (default: 10)
-  --n_jobs int            (default: 1)
-  --precompute_distances str (default: auto)
-  --random_state NoneType (default: None)
-  --tol float             (default: 0.0001)
-  --verbose int           (default: 0)

## AgglomerativeClustering
A type of hierarchical clustering. Several different linkage strategies can be used: ward, complete and average. The number of clusters must be specified and it scales well with large numbers of samples and clusters. Additionally, a connectivity constraint can be given indicating neighboring samples, which avoids overlapping of clusters where it is not possible according to a-priori information. When not using ward linkage, the distance metric used internally can furthermore be specified from several predefined (e.g. Euclidian, Manhattan) or user specified distance measures.

estimator parameters:
-  --affinity str          (default: euclidean)
-  --compute_full_tree str (default: auto)
-  --connectivity NoneType (default: None)
-  --linkage str           (default: ward)
-  --memory Memory         (default: Memory(cachedir=None))
-  --n_clusters int        (default: 2)
-  --pooling_func function (default: mean)

## GaussianMixture
The gaussian mixture model is the only conventional modelling method provided in the *scikit-learn* library that clusters data, thus requiring a model to be trained for use in classification, instead of directly clustering incoming data. Different algorithms for estimating the gaussians are implemented, most prominently the expectation-maximization algorithm. It initializes a specified number of gaussian components with a specified method, and iteratively tries to maximize the likelihood of the training data given the assigned gaussians. Furthermore, a variational Bayesian inference variant is implemented, which extends the expectation-maximization method by incorporating history from previous likelihood distributions.

estimator parameters:
-  --covariance_type str (default: full)
-  --init_params str       (default: kmeans)
-  --max_iter int          (default: 100)
-  --means_init NoneType (default: None)
-  --n_components int      (default: 1)
-  --n_init int            (default: 1)
-  --precisions_init NoneType (default: None)
-  --random_state NoneType (default: None)
-  --reg_covar float       (default: 1e-06)
-  --tol float             (default: 0.001)
-  --verbose int           (default: 0)
-  --verbose_interval int (default: 10)
-  --warm_start bool       (default: False)
-  --weights_init NoneType (default: None)

## DBSCAN
DBSCAN is a popular density-based clustering algorithm. It separates high-density areas with respect to low-density areas by searching for core samples given two parameters, the minimum number of neighborhood samples and the minimum distance between two samples which makes them neighbors. It is a very efficient method for large data sets, however it relies on the fact that clusters are clearly separated by low-density areas, which is rarely the case in activity recognition. In the non-deterministic case of a non-core sample possibly belonging to two clusters, the *scikit-learn* documentation notes that it will be assigned to the first generated cluster, although this order is random.

estimator parameters:
-  --algorithm str         (default: auto)
-  --eps float             (default: 0.5)
-  --leaf_size int         (default: 30)
-  --metric str            (default: euclidean)
-  --min_samples int       (default: 5)
-  --n_jobs int            (default: 1)
-  --p NoneType            (default: None)




# EXAMPLES

## clustering output
The **_train-skl_** module provides the -p/--prediction option regardless of chosen estimator which will directly output the clustering results for the given data. The model will not be output in any way and is only used internally to produce the cluster predictions. The example below shows agglomerative clustering with 2 clusters applied to a short sample data stream:

    echo "abc 3
    > abc 3
    > def 5
    > def 5
    > abc 3
    > def 5" | grt train-skl AgglomerativeClustering --n_cluster 2 -p
    abc	1
    abc	1
    def	0
    def	0
    abc	1
    def	0

The output is a [groundtruth prediction] stream. Note that for some clusterers, the assignment of a specific cluster index to a cluster may be ambiguous (this is not the case for the above agglomerative clustering). Keep in mind also that for a GMM clusterer, this direct prediction output works as well but if no test set is specified via the -n/--trainset option the prediction will always be done on the train set.

## k-fold split
A k-fold split variant is available for the -n/--trainset option with an argument [k.x] which will divide the given samples into k equal folds and will select the x'th fold to output as a test set, or use for the -p/--prediction option. The example below shows KMeans clustering on the given samples; The clusterer information is saved in a model file and the third fold of a 3-fold split on the data is output as a test set:

    echo "abc 3
    > abc 3
    > def 5
    > def 5
    > abc 3
    > def 5" | grt train-skl KMeans --n_cluster 2 -f test.km -n 3.3


    abc 3.0
    def 5.0
