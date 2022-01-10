pipeline {
  agent any
  stages {
    stage('Sequence1') {
      parallel {
        stage('Sequence1') {
          steps {
            build 'test-job1'
          }
        }

        stage('Parallel1') {
          steps {
            build 'test-job1'
          }
        }

      }
    }

    stage('Sequence2') {
      steps {
        build 'test-job1'
      }
    }

  }
}