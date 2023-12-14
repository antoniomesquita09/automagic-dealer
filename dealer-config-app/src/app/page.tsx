"use client";
import Head from "next/head";
import styles from "./page.module.css";
import AddIcon from "@mui/icons-material/Add";
import { useContext } from "react";
import { DataContext } from "./data-provider";

export default function Page() {
  const { games } = useContext(DataContext);

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

  return (
    <div className={styles.main}>
      <Head>
        <title>Automatic Card Dealer</title>
      </Head>
      <h1 className={styles.title}>Automatic Card Dealer</h1>
      <div className={styles.content}>
        <div className={styles.grid}>
          {games.map((game, index) => (
            <div
              className={styles.card}
              style={{ backgroundColor: colors[index] }}
              key={index}
              onClick={() => (window.location.href = String(game.id))}
            >
              {game.name}
            </div>
          ))}
          <div
            className={`${styles.card} ${styles.add}`}
            onClick={() => (window.location.href = "/new")}
          >
            <AddIcon />
          </div>
        </div>
      </div>
    </div>
  );
}
