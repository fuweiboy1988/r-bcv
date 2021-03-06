% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/gabriel.cor.correct.R
\name{gabriel.cor.correct}
\alias{gabriel.cor.correct}
\title{Choosing the number of clusters of a matrix using correlation-corrected Gabriel CV method.}
\usage{
gabriel.cor.correct(x, krow = 5, kcol = 2, maxcenters = 10,
  classify.method = "nearest")
}
\arguments{
\item{x}{The matrix to find the number of clusters.}

\item{krow}{The number of row folds.}

\item{kcol}{The number of column folds.}

\item{maxcenters}{The upper bound of searching range, i.e k searched in 1:maxcenters}

\item{classify.method}{The classifier used inside the algorithm,
with possible choice "nearest", "lda-equal" and "lda-proportions".}
}
\value{
The number of clusters chosen by the algorithm.
}
\description{
Perform correlation-corrected version of Gabriel cross-validation for determining the
number of clusters of a matrix
}
\details{
This function is a correlation-corrected verison of \code{\link{cv.kmeans.gabriel}}.
The \code{\link{cv.kmeans.gabriel}} works well when the correlation between the dimensions of
matrix \eqn{x} is low. It tends to overestimate the number of clusters \eqn{k} when the
correlation between dimension is high. This function overcomes such difficulty by
rotating the original matrix so that the correlation between dimensions is diminished.
It then calls the \code{\link{cv.kmeans.gabriel}} on the rotated matrix to find the number
\eqn{k}. Simulation shows this function works well when all the clusters have similar
covariance structure.
}
\examples{
## generate a 100x2 matrix with single cluster centered at (2,1)
## the correlation rho between dimensions is 0.6
library(MASS)
sigma = matrix(c(1,0.6,0.6,1),nrow = 2, ncol=2)
x <- mvrnorm(n = 100, mu = c(2,1), sigma)

## the k returned by cv.kmeans.gabriel function
cv.kmeans.gabriel(x, 5, 2, maxcenters=10, classify.method="nearest")$$centers

## the k returned by correlation-corrected gabriel.cor.correct function
gabriel.cor.correct(x, 5, 2, maxcenters=10, classify.method="nearest")

}
\author{
Wei Fu, Patrick O. Perry
}

