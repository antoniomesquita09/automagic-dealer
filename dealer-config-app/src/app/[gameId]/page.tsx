"use client";
import React, { useContext, useEffect, useState } from "react";
import { toast } from "react-toastify";
import styles from "./page.module.css";
import { useRouter } from "next/navigation";
import Image from "next/image";
import {
  TextField,
  Button,
  Dialog,
  DialogTitle,
  DialogContent,
  DialogContentText,
  DialogActions,
  ToggleButtonGroup,
  ToggleButton,
  InputAdornment,
  Checkbox,
} from "@mui/material";
import ArrowBackIcon from "@mui/icons-material/ArrowBack";
import AddIcon from "@mui/icons-material/Add";
import GroupsIcon from "@mui/icons-material/Groups";
import TableBarIcon from "@mui/icons-material/TableBar";
import DeleteIcon from "@mui/icons-material/Delete";
import { DataContext } from "../data-provider";
import axios from "axios";

export type InstrucionProps = {
  type: "players" | "table";
  cardAmount: number;
};
export default function Page({ params }: { params: { gameId: string } }) {
  const { games, setGames } = useContext(DataContext);
  let initInstructions: InstrucionProps[];
  let game;

  const router = useRouter();

  const [name, setName] = useState("");
  const [minPlayers, setMinPlayers] = useState("");
  const [maxPlayers, setMaxPlayers] = useState("");
  const [instructions, setInstructions] = useState<InstrucionProps[]>([]);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [deleteInstructionDialogOpen, setDeleteInstructionDialogOpen] =
    useState(false);
  const [deleteGameDialogOpen, setDeleteGameDialogOpen] = useState(false);
  const [instructionEdit, setInstructionEdit] = useState<InstrucionProps>({
    type: "players",
    cardAmount: 1,
  });
  const [indexToBeEdited, setIndexToBeEdited] = useState<number>();
  const [indexToBeDeleted, setIndexToBeDeleted] = useState<number>();
  const [excludedCards, setExcludedCards] = useState<number[]>([]);

  useEffect(() => {
    if (params.gameId === "new") {
      initInstructions = [];
    } else {
      game = games.filter((game) => game.id === params.gameId)[0];
      initInstructions = game?.instructions;
      setName(game?.name);
      setMinPlayers(String(game?.minPlayers));
      setMaxPlayers(String(game?.maxPlayers));
      setExcludedCards(game?.excludedCards);
    }
    setInstructions(initInstructions);
  }, [games, params.gameId]);

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    console.log({ name, minPlayers, maxPlayers });
  };

  const colors = [
    "#39a3ff",
    "#ff8027",
    "#98c863",
    "#1d2023",
    "#37b8c0",
    "#ff3226",
    "#fdb854",
    "#40c065",
    "#ff3b30",
    "#2d3f48",
  ];

  const suits = ["clubs.webp", "hearts.webp", "spades.png", "diamonds.png"];

  const cardValues = [
    "A",
    "K",
    "Q",
    "J",
    "T",
    "9",
    "8",
    "7",
    "6",
    "5",
    "4",
    "3",
    "2",
  ];

  const handleTypeChange = (
    event: React.MouseEvent<HTMLElement>,
    newType: "players" | "table"
  ) => {
    setInstructionEdit({ ...instructionEdit, type: newType });
  };

  const handleCancelInstruction = () => {
    setDialogOpen(false);
    setInstructionEdit({ type: "players", cardAmount: 1 });
  };

  const handleDeleteGame = async () => {
    try {
      await axios.delete(`http://127.0.0.1:5000/games/${params.gameId}`);
      const newGames = games.filter((game) => game.id !== params.gameId);
      setGames(newGames);
      router.push("/");
      toast.success("Jogo excluído com sucesso!");
    } catch (error) {
      console.error("Failed to delete game:", error);
      toast.error("Erro ao excluir o jogo.");
      setDeleteGameDialogOpen(false);
    }
  };

  const handleDeleteInstruction = () => {
    setInstructions(
      instructions.filter((_, index) => index !== indexToBeDeleted)
    );
    setDeleteInstructionDialogOpen(false);
  };

  const handleOpenEditInstruction = (index: number) => {
    setInstructionEdit(instructions[index]);
    setIndexToBeEdited(index);
    setDialogOpen(true);
  };

  const handleEditInstruction = () => {
    if (indexToBeEdited === instructions.length) {
      setInstructions([...instructions, instructionEdit]);
    } else {
      setInstructions(
        instructions.map((instruction, index) =>
          index === indexToBeEdited ? instructionEdit : instruction
        )
      );
    }
    setDialogOpen(false);
    setInstructionEdit({ type: "players", cardAmount: 1 });
    setIndexToBeEdited(undefined);
  };

  const handleExcludedCard = (cardIndex: number) => {
    if (excludedCards.includes(cardIndex)) {
      setExcludedCards(excludedCards.filter((number) => number !== cardIndex));
    } else {
      setExcludedCards([...excludedCards, cardIndex]);
    }
  };
  const handleSubmitGame = async () => {
    if (params.gameId === "new") {
      try {
        await axios.post("http://127.0.0.1:5000/games", {
          name: name,
          minPlayers: minPlayers,
          maxPlayers: maxPlayers,
          instructions: instructions,
          excludedCards: excludedCards,
        });
        setGames([
          ...games,
          {
            name: name,
            minPlayers: parseInt(minPlayers),
            maxPlayers: parseInt(maxPlayers),
            instructions: instructions,
            excludedCards: excludedCards,
            id: "new",
          },
        ]);
        router.push("/");
        toast.success("Jogo criado com sucesso!");
      } catch (error) {
        console.error("Failed to create game:", error);
        toast.error("Erro ao criar o jogo.");
      }
    } else {
      try {
        await axios.put(`http://127.0.0.1:5000/games/${params.gameId}`, {
          name: name,
          minPlayers: minPlayers,
          maxPlayers: maxPlayers,
          instructions: instructions,
        });
        router.push("/");
        toast.success("Jogo editado com sucesso!");
      } catch (error) {
        console.error("Failed to edit game:", error);
        toast.error("Erro ao editar o jogo.");
      }
    }
  };

  return (
    <div className={styles.main}>
      <div className={styles.header}>
        <a className={styles.back} href="/">
          <ArrowBackIcon />
        </a>
        <h1 className={styles.title}>
          {params.gameId === "new" ? "Novo" : "Editar"} Jogo
        </h1>
        {params.gameId === "new" ? (
          <div></div>
        ) : (
          <div
            className={styles.deleteGame}
            onClick={(e) => {
              setDeleteGameDialogOpen(true);
              e.stopPropagation();
            }}
          >
            <DeleteIcon />
          </div>
        )}
      </div>
      <form className={styles.form} onSubmit={handleSubmit}>
        <div className={styles.top}>
          <TextField
            id="name"
            label="Nome"
            type="text"
            value={name}
            inputProps={{
              maxLength: 20,
            }}
            onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
              setName(e.target.value)
            }
            className={styles.input}
            variant="filled"
            fullWidth
            required
          />
          <TextField
            id="minPlayers"
            label="Mínimo de jogadores"
            type="number"
            inputProps={{
              min: 1,
            }}
            value={minPlayers}
            onChange={(e: React.ChangeEvent<HTMLInputElement>) => {
              setMinPlayers(e.target.value);
              if (e.target.value > maxPlayers) setMaxPlayers(e.target.value);
            }}
            className={styles.input}
            variant="filled"
            fullWidth
            required
          />
          <TextField
            id="maxPlayers"
            label="Máximo de jogadores"
            type="number"
            inputProps={{
              min: minPlayers,
            }}
            value={maxPlayers}
            onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
              setMaxPlayers(e.target.value)
            }
            className={styles.input}
            variant="filled"
            fullWidth
            required
          />
        </div>
        <div className={styles.cardSelection}>
          {suits.map((suit, indexSuit) => (
            <div className={styles.cardRow} key={suit}>
              {cardValues.map((value, indexValue) => (
                <button
                  type="button"
                  key={value}
                  className={styles.cardButton}
                  onClick={() =>
                    handleExcludedCard(indexSuit * 13 + indexValue)
                  }
                >
                  {value}
                  <Image src={`/${suit}`} width={20} height={20} alt="" />
                  <span
                    className={`${styles.excluded} ${
                      excludedCards?.includes(indexSuit * 13 + indexValue)
                        ? styles.active
                        : ""
                    }`}
                  >
                    ❌
                  </span>
                </button>
              ))}
            </div>
          ))}
        </div>
        {instructions?.map((instruction, index) => (
          <div
            className={styles.card}
            style={{ backgroundColor: colors[index] }}
            key={index}
            onClick={() => handleOpenEditInstruction(index)}
          >
            <span>{index + 1}ª distribuição</span>
            <div className={styles.cardInfo}>
              <span>
                {instruction.cardAmount} carta
                {instruction.cardAmount > 1 && "s"}{" "}
                {instruction.type === "players"
                  ? "para cada jogador"
                  : "na mesa"}
              </span>
              <div
                className={styles.deleteCard}
                onClick={(e) => {
                  setDeleteInstructionDialogOpen(true);
                  setIndexToBeDeleted(index);
                  e.stopPropagation();
                }}
              >
                <DeleteIcon />
              </div>
            </div>
          </div>
        ))}
        <div
          className={`${styles.card} ${styles.add}`}
          onClick={() => {
            setDialogOpen(true);
            setIndexToBeEdited(instructions.length);
          }}
        >
          <AddIcon />
        </div>

        <div className={styles.buttonContainer}>
          <Button
            variant="contained"
            size="large"
            type="submit"
            className={styles.submit}
            onClick={handleSubmitGame}
          >
            Salvar
          </Button>
        </div>
      </form>
      <Dialog open={dialogOpen} onClose={() => setDialogOpen(false)}>
        <DialogTitle id="alert-dialog-title">
          {indexToBeEdited ? indexToBeEdited + 1 : 1}ª distribuição
        </DialogTitle>
        <DialogContent>
          <div className={styles.type}>
            <span style={{ flex: 1, textAlign: "right", fontWeight: 500 }}>
              Jogadores
            </span>
            <ToggleButtonGroup
              style={{ flex: 0, margin: "0 15px" }}
              value={instructionEdit.type}
              exclusive
              onChange={handleTypeChange}
              aria-label="text alignment"
            >
              <ToggleButton value="players" aria-label="left aligned">
                <GroupsIcon />
              </ToggleButton>
              <ToggleButton value="table" aria-label="right aligned">
                <TableBarIcon />
              </ToggleButton>
            </ToggleButtonGroup>
            <span style={{ flex: 1, fontWeight: 500 }}>Mesa</span>
          </div>
          <div className={styles.amount}>
            <TextField
              id="cardAmount"
              type="number"
              inputProps={{
                min: 1,
              }}
              value={instructionEdit.cardAmount}
              onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
                setInstructionEdit({
                  ...instructionEdit,
                  cardAmount: Number(e.target.value),
                })
              }
              style={{ width: "335px" }}
              InputProps={{
                endAdornment: (
                  <InputAdornment position="end">
                    carta{instructionEdit.cardAmount > 1 && "s"}{" "}
                    {instructionEdit.type === "players"
                      ? "por jogador"
                      : "na mesa"}
                  </InputAdornment>
                ),
              }}
              variant="outlined"
              required
            />
          </div>
        </DialogContent>
        <DialogActions>
          <Button onClick={handleCancelInstruction}>Cancelar</Button>
          <Button onClick={handleEditInstruction} autoFocus>
            OK
          </Button>
        </DialogActions>
      </Dialog>
      <Dialog
        open={deleteInstructionDialogOpen}
        onClose={() => setDeleteInstructionDialogOpen(false)}
      >
        <DialogContent>
          <DialogContentText id="alert-dialog-description">
            Deseja realmente excluir esta distribuição?
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setDeleteInstructionDialogOpen(false)}>
            Cancelar
          </Button>
          <Button onClick={handleDeleteInstruction} autoFocus>
            EXCLUIR
          </Button>
        </DialogActions>
      </Dialog>
      <Dialog
        open={deleteGameDialogOpen}
        onClose={() => setDeleteGameDialogOpen(false)}
      >
        <DialogContent>
          <DialogContentText id="alert-dialog-description">
            Deseja realmente excluir este jogo?
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setDeleteGameDialogOpen(false)}>
            Cancelar
          </Button>
          <Button onClick={handleDeleteGame} autoFocus>
            EXCLUIR
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}
