FROM python:3.12-slim
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends build-essential git \
    && rm -rf /var/lib/apt/lists/*
COPY . .
# Two incompatible dependency sets (numpy>=2 vs numpy<2 for torch==2.2.2) -- reproduce.sh
# creates both venvs (.venv, .venv_codebert) and routes each pipeline step to the right one.
RUN chmod +x reproduce.sh
CMD ["./reproduce.sh"]
