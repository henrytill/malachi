FROM mcr.microsoft.com/devcontainers/base:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -qq && \
    apt-get install -yqq python3-platformdirs python3-fitz python3-pygit2 python3-typeshed \
    black mypy pylint python3-flake8 python3-isort python3-venv

USER vscode
