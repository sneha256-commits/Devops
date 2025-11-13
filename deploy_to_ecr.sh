[200~#!/bin/bash

# -------- CONFIGURATION --------
IMAGE_NAME="c-http-service"
ECR_REPO_NAME="c-http-service"
AWS_REGION="us-east-1"
CONTAINER_NAME="microservice-container"

# Get AWS Account ID
ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)

# Full ECR URI
ECR_URI="$ACCOUNT_ID.dkr.ecr.$AWS_REGION.amazonaws.com/$ECR_REPO_NAME"

# -------- BUILD DOCKER IMAGE --------
echo "Building Docker image..."
docker build -t $IMAGE_NAME .

# -------- CREATE ECR REPO (if not exists) --------
echo "Checking/creating ECR repository..."
aws ecr describe-repositories --repository-names $ECR_REPO_NAME --region $AWS_REGION >/dev/null 2>&1
if [ $? -ne 0 ]; then
	    aws ecr create-repository --repository-name $ECR_REPO_NAME --region $AWS_REGION
	        echo "ECR repository created: $ECR_URI"
	else
		    echo "ECR repository exists: $ECR_URI"
fi

# -------- LOGIN TO ECR --------
echo "Logging in to ECR..."
aws ecr get-login-password --region $AWS_REGION | docker login --username AWS --password-stdin $ECR_URI

# -------- TAG IMAGE --------
echo "Tagging image for ECR..."
docker tag $IMAGE_NAME:latest $ECR_URI:latest

# -------- PUSH IMAGE --------
echo "Pushing image to ECR..."
docker push $ECR_URI:latest

# -------- RUN CONTAINER --------
echo "Stopping existing container (if any)..."
docker rm -f $CONTAINER_NAME >/dev/null 2>&1

echo "Running container on port 8080..."
docker run -d --name $CONTAINER_NAME -p 8080:8080 $ECR_URI:latest

echo "Deployment complete! Access your service at EC2_PUBLIC_IP:8080"

