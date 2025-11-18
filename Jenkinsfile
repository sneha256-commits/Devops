pipeline {
    agent any

    environment {
        IMAGE_NAME      = "c-http-service"
        ECR_REPO_NAME   = "c-http-service"
        AWS_REGION      = "us-east-1"
        CONTAINER_NAME  = "microservice-container"
        HOST_PORT       = "9090"               // Host port for the service
        CONTAINER_PORT  = "8080"               // Container port
        AWS_CREDENTIAL  = "aws-ecr-creds"     // Jenkins AWS credentials ID
    }

    stages {

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build Docker Image') {
            steps {
                sh '''
                    echo "Building Docker image..."
                    docker build -t $IMAGE_NAME .
                '''
            }
        }

        stage('Create ECR Repository') {
            steps {
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding',
                                  credentialsId: AWS_CREDENTIAL]]) {
                    sh '''
                        ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
                        ECR_URI="$ACCOUNT_ID.dkr.ecr.$AWS_REGION.amazonaws.com/$ECR_REPO_NAME"
                        echo "Checking if ECR repository exists..."
                        aws ecr describe-repositories --repository-names $ECR_REPO_NAME --region $AWS_REGION >/dev/null 2>&1
                        if [ $? -ne 0 ]; then
                            aws ecr create-repository --repository-name $ECR_REPO_NAME --region $AWS_REGION
                            echo "ECR repository created: $ECR_URI"
                        else
                            echo "ECR repository exists: $ECR_URI"
                        fi
                    '''
                }
            }
        }

        stage('Login to ECR') {
            steps {
                withCredentials([[$class: 'AmazonWebServicesCredentialsBinding',
                                  credentialsId: AWS_CREDENTIAL]]) {
                    sh '''
                        ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
                        ECR_URI="$ACCOUNT_ID.dkr.ecr.$AWS_REGION.amazonaws.com/$ECR_REPO_NAME"
                        echo "Logging in to ECR..."
                        aws ecr get-login-password --region $AWS_REGION \
                            | docker login --username AWS --password-stdin $ECR_URI
                    '''
                }
            }
        }

        stage('Tag and Push Image') {
            steps {
                sh '''
                    ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
                    ECR_URI="$ACCOUNT_ID.dkr.ecr.$AWS_REGION.amazonaws.com/$ECR_REPO_NAME"
                    echo "Tagging image for ECR..."
                    docker tag $IMAGE_NAME:latest $ECR_URI:latest
                    echo "Pushing image to ECR..."
                    docker push $ECR_URI:latest
                '''
            }
        }

        stage('Deploy Container') {
            steps {
                sh '''
                    echo "Stopping existing container (if any)..."
                    docker rm -f $CONTAINER_NAME >/dev/null 2>&1
                    echo "Running container on host port $HOST_PORT..."
                    ACCOUNT_ID=$(aws sts get-caller-identity --query Account --output text)
                    ECR_URI="$ACCOUNT_ID.dkr.ecr.$AWS_REGION.amazonaws.com/$ECR_REPO_NAME"
                    docker run -d --name $CONTAINER_NAME -p $HOST_PORT:$CONTAINER_PORT $ECR_URI:latest
                '''
            }
        }
    }

    post {
        success {
            echo "Deployment complete! Access your service at EC2_PUBLIC_IP:$HOST_PORT"
        }
        failure {
            echo "Pipeline failed."
        }
    }
}
