# Contributing to MusicEngine

Thank you for your interest in contributing your time and code to MusicEngine! To ensure the project's code quality and maintain an efficient collaboration process, please read the following guidelines carefully before you submit any code.

## Workflow

The core development of this project takes place on the `develop` branch. The `main` branch is reserved for stable, tested, official release versions. Therefore, **all Pull Requests (PRs) must be submitted to the `develop` branch**.

### Steps to Submit a PR

1.  **Fork the Repository**
    Click the "Fork" button in the upper-right corner of the project's homepage to create your own copy of the repository.

2.  **Clone Your Fork**
    ```bash
    git clone --recursive [https://github.com/YOUR_USERNAME/MusicEngine.git](https://github.com/YOUR_USERNAME/MusicEngine.git)
    cd MusicEngine
    ```

3.  **Switch to the `develop` Branch**
    In your local repository, switch from `main` to the `develop` branch to get the latest development code.
    
    ```bash
    git checkout develop
    ```
    
4.  **Create Your Feature Branch**
    Create your new branch from the `develop` branch. Please use a descriptive branch name, for example, `feature/add-playback-speed-control`.
    ```bash
    git checkout -b feature/your-feature-name
    ```

5.  **Code and Commit**
    Make your code changes on the new branch and commit them.
    ```bash
    git add .
    git commit -m "feat: Add feature XXX"
    ```

6.  **Push to Your Fork**
    Push your feature branch to your GitHub Fork repository.
    ```bash
    git push origin feature/your-feature-name
    ```

7.  **Create a Pull Request**
    On your Fork's GitHub page, click "Contribute" -> "Open pull request".
    At this point, please ensure the following:
    * **base repository** is `haoruanwn/MusicEngine`
    * **base** branch is set to **`develop`**
    * **head repository** is `YOUR_USERNAME/MusicEngine`
    * **compare** branch is set to `feature/your-feature-name`

    Then, fill in the PR title and description to explain your changes, and finally, click "Create pull request".

Thank you for your understanding and cooperation!