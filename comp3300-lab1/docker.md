# Docker image for comp3300

Install [Docker Desktop](https://docs.docker.com/desktop/) if you haven't done so. 

# Creating a docker container and shared folder. 

The following need to be run once only. 

- Download the docker image for comp3300: 

    ```
    docker pull atiu2019/anu-comp3300
    ```

- Create a container and configure a shared folder (replace `/path/to/shared_folder` with the path you want to share with the container).

    ```
    docker create --platform=linux/amd64 -it --cpus=2 -m 4g --name=comp3300 -v /root/comp3300/:/home/osilab/ atiu2019/anu-comp3300
    ```


# Running the container 

- Starting the container

    ```
    docker start comp3300
    ```

- Connecting to the container

    ```
    docker exec -it -u root -w /home/osilab comp3300 bash
    ```

- Stopping the container

    ```
    docker stop comp3300
    ```

