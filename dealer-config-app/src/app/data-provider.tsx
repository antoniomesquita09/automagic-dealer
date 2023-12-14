"use client";
import axios from "axios";
import { createContext, useEffect, useState } from "react";
import { InstrucionProps } from "./[gameId]/page";

export type GameProps = {
  name: string;
  id: string;
  minPlayers: number;
  maxPlayers: number;
  instructions: InstrucionProps[];
  excludedCards: number[];
};

type DataContextType = {
  games: GameProps[];
  setGames: React.Dispatch<React.SetStateAction<GameProps[]>>;
  fetchGames: () => Promise<void>;
};

export const DataContext = createContext<DataContextType>({
  games: [],
  setGames: () => {},
  fetchGames: async () => {},
});

export default function DataProvider({ children }: any) {
  const [games, setGames] = useState<GameProps[]>([]);

  const fetchGames = async () => {
    try {
      const response = await axios.get("http://127.0.0.1:5000/games"); // Your API endpoint to fetch games
      setGames(response.data);
    } catch (error) {
      console.error("Failed to fetch games:", error);
    }
  };

  useEffect(() => {
    fetchGames();
  }, []);

  return (
    <DataContext.Provider value={{ games, setGames, fetchGames }}>
      {children}
    </DataContext.Provider>
  );
}
